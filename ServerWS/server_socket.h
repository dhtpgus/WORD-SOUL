//---------------------------------------------------
// 
// server_socket.h - server::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <vector>
#include <fstream>
#include <array>
#include "accepter.h"
#include "packet.h"
#include "party.h"

namespace server {
	class Socket {
	public:
		Socket() noexcept : threads_{}, accepter_{},
			clients_{ std::make_shared<ClientArray>(GetMaxClients(), thread::GetNumWorker()) },
			parties_(GetMaxClients() / 2) {
			WSAData wsadata;
			if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
				exit(1);
			}
			memset(&server_addr_, 0, sizeof(server_addr_));
			server_addr_.sin_family = AF_INET;
			server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
			server_addr_.sin_port = htons(kPort);

			accepter_ = std::make_shared<Accepter>();

			iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
			accepter_->LinkIOCP(iocp_);

			for (Party& party : parties_) {
				party.InitEntityManager(100, thread::GetNumWorker());
			}
		}
		Socket(const Socket&) = delete;
		Socket(Socket&&) = delete;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = delete;
		~Socket() noexcept {
			WSACleanup();
		}

		void Start() noexcept {
			accepter_->BindAndListen(server_addr_);
			accepter_->Accept();

			std::print("[Info] Server Starts\n");
			CreateThread();
		}

		void Disconnect(int id) noexcept {
			auto& party = parties_[(*clients_)[id].GetPartyID()];
			party.Exit(id);
			auto partner_id = party.GetPartnerID(id);
			if (clients_->TryAccess(partner_id)) {
				(*clients_)[partner_id].Push<packet::SCRemoveEntity>(entity::kPartnerID);
				clients_->EndAccess(partner_id);
			}
			clients_->ReserveDelete(id);
		}

		int GetMaxClients() const noexcept {
			static int max_clients;
			static bool has_read_file;
			if (has_read_file) {
				return max_clients;
			}

			std::string file_name{ "max_clients.txt" };
			std::ifstream in{ std::format("data/{}", file_name) };
			if (not in) {
				in.open(std::format("../../data/{}", file_name));
				if (not in) {
					std::print("[Error] Cannot Open file: data/{}\n", file_name);
					exit(1);
				}
			}

			in >> max_clients;
			has_read_file = true;

			return max_clients;
		}
	private:
		using ClientArray = lf::Array<client::Session>;
		void CreateThread() noexcept {
			threads_.reserve(thread::GetNumWorker() + 1);
			for (int i = 0; i < thread::GetNumWorker(); ++i) {
				threads_.emplace_back([this, i]() { WorkerThread(i); });
			};

			for (std::thread& th : threads_) {
				th.join();
			}
		}

		void WorkerThread(int id) noexcept;
		void ProcessAccept() noexcept;

		void ProcessPacket(BufferRecv& buf, DWORD& n_bytes, int session_id) noexcept
		{
			if (n_bytes == 0) {
				return;
			}

			packet::Size size = *(packet::Size*)(buf.GetData());
			if (n_bytes < sizeof(size) + sizeof(packet::Type) + size) {
				buf.SaveRemains(n_bytes);
				n_bytes = 0;
				return;
			}

			packet::Type type = *(packet::Type*)(buf.GetData() + 1);
			n_bytes -= sizeof(size) + sizeof(type) + size;

			switch (type) {
			case packet::Type::kTest: {
				packet::Test p{ buf.GetData() };
				std::print("{} {} {}, {}\n", p.a, p.b, p.c, n_bytes);
				break;
			}
			case packet::Type::kCSJoinParty: {
				packet::CSJoinParty p{ buf.GetData() };
				if (parties_[p.id].TryEnter(session_id)) {
					if (debug::DisplaysMSG()) {
						std::print("ID: {} has joined Party: {}.\n", session_id, p.id);
					}

					(*clients_)[session_id].Push<packet::SCResult>(true);
					(*clients_)[session_id].SetPartyID(p.id);
					auto partner_id = parties_[p.id].GetPartnerID(session_id);

					if ((*clients_).TryAccess(partner_id)) {
						auto& pos = (*clients_)[session_id].GetPlayer().GetPostion();
						(*clients_)[partner_id].Push<packet::SCNewEntity>(
							entity::kPartnerID, pos.x, pos.y, pos.z, entity::Type::kPlayer);
						(*clients_).EndAccess(partner_id);
					}
				}
				else {
					(*clients_)[session_id].Push<packet::SCResult>(false);
				}
				break;
			}
			case packet::Type::kCSPosition: {
				packet::CSPosition p{ buf.GetData() };
				(*clients_)[session_id].GetPlayer().SetPosition(p.x, p.y, p.z);
				auto party_id{ (*clients_)[session_id].GetPartyID() };
				auto partner_id = parties_[party_id].GetPartnerID(session_id);

				if ((*clients_).TryAccess(partner_id)) {
					(*clients_)[partner_id].Push<packet::SCPosition>(entity::kPartnerID, p.x, p.y, p.z);
					(*clients_).EndAccess(partner_id);
				}
				break;
			}
			case packet::Type::kCSLeaveParty: {
				auto party_id = (*clients_)[session_id].GetPartyID();
				if (party_id < 0 or party_id >= parties_.size()) {
					break;
				}
				parties_[party_id].Exit(session_id);
				break;
			}
			default: {
				std::print("[Error] Unknown Packet: {}\n", static_cast<int>(type));
				exit(1);
			}
			}

			if (n_bytes == 0) {
				buf.ResetCursor();
			}
			else {
				buf.MoveCursor(sizeof(size) + sizeof(type) + size);
			}
		}

		static constexpr unsigned short kPort{ 21155 };
		static constexpr auto kTransferFrequency{ 1.0 / 100.0 };

		std::vector<std::thread> threads_;
		sockaddr_in server_addr_;
		HANDLE iocp_;
		std::shared_ptr<Accepter> accepter_;
		std::shared_ptr<ClientArray> clients_;
		std::vector<Party> parties_;
	};

	extern Socket sock;
};
