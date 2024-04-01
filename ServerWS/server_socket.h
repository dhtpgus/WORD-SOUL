//---------------------------------------------------
// 
// server_socket.h - server::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <vector>
#include <fstream>
#include <array>
#include "packet.h"
#include "session.h"
#include "party.h"

namespace server {
	class Socket {
	public:
		Socket() : threads_{}, accept_over_{ Operation::kAccept }, accept_sock_{},
			clients_{ std::make_shared<ClientArray>(GetMaxClients(), thread::GetNumWorker()) },
			parties_(GetMaxClients() / 2) {
			if (WSAStartup(MAKEWORD(2, 2), &wsa_) != 0) {
				exit(1);
			}

			listen_sock_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			accept_sock_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (listen_sock_ == INVALID_SOCKET) {
				exit(1);
			}
			
			memset(&server_addr_, 0, sizeof(server_addr_));
			server_addr_.sin_family = AF_INET;
			server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
			server_addr_.sin_port = htons(kPort);

			accept_over_.op = Operation::kAccept;

			iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_sock_), iocp_, 9999, 0);

			for (Party& party : parties_) {
				party.InitEntityManager(100, thread::GetNumWorker());
			}
		}
		Socket(const Socket&) = delete;
		Socket(Socket&&) = delete;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = delete;
		~Socket() {
			closesocket(listen_sock_);
			WSACleanup();
		}

		void Start() {
			Bind();
			Listen();

			int addr_len = sizeof(sockaddr_in);
			AcceptEx(listen_sock_, accept_sock_, accept_over_.buf, 0,
				addr_len + 16, addr_len + 16, 0, &accept_over_.over);

			std::print("[Info] Server Starts\n");
			CreateThread();
		}

		void Disconnect(int id) {
			auto& party = parties_[(*clients_)[id].GetPartyID()];
			party.Exit(id);
			auto partner_id = party.GetPartnerID(id);
			if (clients_->TryAccess(partner_id)) {
				(*clients_)[partner_id].Push<packet::SCRemoveEntity>(-1);
				clients_->EndAccess(partner_id);
			}
			clients_->ReserveDelete(id);
		}

		int GetMaxClients() const {
			static int max_clients;
			static bool has_read_file;
			if (has_read_file) {
				return max_clients;
			}
			std::ifstream in{ "data/max_clients.txt" };
			if (not in) {
				in.open("../../data/max_clients.txt");
				if (not in) {
					std::print("[Error] Cannot Open file: data/max_client.txt");
					exit(1);
				}
			}
			in >> max_clients;
			has_read_file = true;

			return max_clients;
		}
	private:
		using ClientArray = lf::Array<client::Session>;

		void Bind() {
			if (bind(listen_sock_, (sockaddr*)&server_addr_, sizeof(server_addr_)) == SOCKET_ERROR) {
				exit(1);
			}
		}
		void Listen() {
			if (listen(listen_sock_, SOMAXCONN) == SOCKET_ERROR) {
				exit(1);
			}
		}
		void CreateThread() {
			threads_.reserve(thread::GetNumWorker() + 1);
			for (int i = 0; i < thread::GetNumWorker(); ++i) {
				threads_.emplace_back([this, i]() { WorkerThread(i); });
			};

			for (std::thread& th : threads_) {
				th.join();
			}
		}

		void WorkerThread(int id);
		void ProcessAccept();

		void Deserialize(char*& bytes, DWORD& n_bytes, int session_id)
		{
			if (n_bytes <= 0) {
				return;
			}

			packet::Size size = *(packet::Size*)(bytes++);
			packet::Type type = *(packet::Type*)(bytes++);

			n_bytes -= sizeof(size) + sizeof(type) + size;

			switch (type) {
			case packet::Type::kTest: {
				auto p{ std::make_unique<packet::Test>(bytes) };
				std::print("{} {} {}\n", p->a, p->b, p->c);
				break;
			}
			case packet::Type::kCSJoinParty: {
				auto p{ std::make_unique<packet::CSJoinParty>(bytes) };
				if (parties_[p->id].TryEnter(session_id)) {
					if (debug::IsDebugMode()) {
						std::print("{}가 {}번 파티에 입장.\n", session_id, p->id);
					}

					(*clients_)[session_id].Push<packet::SCResult>(true);
					(*clients_)[session_id].SetPartyID(p->id);
					auto partner_id = parties_[p->id].GetPartnerID(session_id);

					if ((*clients_).TryAccess(partner_id)) {
						auto& pos = (*clients_)[session_id].GetPlayer().GetPostion();
						(*clients_)[partner_id].Push<packet::SCNewEntity>(
							-1, pos.x, pos.y, pos.z, entity::Type::kPlayer);
						(*clients_).EndAccess(partner_id);
					}
				}
				else {
					(*clients_)[session_id].Push<packet::SCResult>(false);
				}
				break;
			}
			case packet::Type::kCSPosition: {
				auto p{ std::make_unique<packet::CSPosition>(bytes) };
				(*clients_)[session_id].GetPlayer().SetPosition(p->x, p->y, p->z);
				auto party_id{ (*clients_)[session_id].GetPartyID() };
				auto partner_id = parties_[party_id].GetPartnerID(session_id);

				if ((*clients_).TryAccess(partner_id)) {
					(*clients_)[partner_id].Push<packet::CSPosition>(bytes);
					(*clients_).EndAccess(partner_id);
				}
				break;
			}
			default: {
				std::print("[Error] Unknown Packet: {}\n", (int)type);
				exit(1);
			}
			}
		}

		static constexpr unsigned short kPort{ 21155 };
		static constexpr size_t kBufferSize{ 1024 };

		WSADATA wsa_;
		std::vector<std::thread> threads_;
		sockaddr_in server_addr_;
		SOCKET listen_sock_;
		SOCKET accept_sock_;
		HANDLE iocp_;
		OverEx accept_over_;
		std::shared_ptr<ClientArray> clients_;
		std::vector<Party> parties_;
	};

	extern Socket sock;
};
