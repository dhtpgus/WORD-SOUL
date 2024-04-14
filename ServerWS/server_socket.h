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
			sessions_{ std::make_shared<SessionArray>(GetMaxClients(), thread::GetNumWorker()) },
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
			auto& party = parties_[(*sessions_)[id].GetPartyID()];
			party.Exit(id);
			auto partner_id = party.GetPartnerID(id);
			if (sessions_->TryAccess(partner_id)) {
				(*sessions_)[partner_id].Push<packet::SCRemoveEntity>(entity::kPartnerID);
				sessions_->EndAccess(partner_id);
			}
			sessions_->ReserveDelete(id);
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
		using SessionArray = lf::Array<client::Session>;
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
		void Send() noexcept {
			//int cnt{};
			for (int i = thread::ID(); i < GetMaxClients(); i += thread::GetNumWorker()) {
				if (sessions_->TryAccess(i)) {
					if (false == (*sessions_)[i].Send()) {
						Disconnect(i);
					}
					//cnt += 1;
					sessions_->EndAccess(i);
				}
			}
			//if (rng.Rand(0, 2000) == 2) std::print("{}: {}\n", thread::ID(), cnt);
		}
		void ProcessPacket(BufferRecv& buf, DWORD& n_bytes, int session_id) noexcept;

		static constexpr unsigned short kPort{ 21155 };
		static constexpr auto kTransferFrequency{ 1.0 / 55555 };

		std::vector<std::thread> threads_;
		sockaddr_in server_addr_;
		HANDLE iocp_;
		std::shared_ptr<Accepter> accepter_;
		std::shared_ptr<SessionArray> sessions_;
		std::vector<Party> parties_;
	};

	extern Socket sock;
};
