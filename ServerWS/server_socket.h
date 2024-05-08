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
#include "timer_thread.h"

namespace server {
	class Socket {
	public:
		Socket() noexcept : threads_{}, accepter_{},
			sessions_{ std::make_shared<SessionArray>(client::GetMaxClients(), thread::kNumWorkers) } {
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

			for (int i = 0; i < parties.size(); ++i) parties[i].SetID(i);
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
			auto& party = parties[(*sessions_)[id].GetPartyID()];
			party.Exit(id);
			auto partner_id = party.GetPartnerID(id);
			if (sessions_->TryAccess(partner_id)) {
				(*sessions_)[partner_id].Push<packet::SCRemoveEntity>(entity::kPartnerID, 0);
				sessions_->EndAccess(partner_id);
			}
			sessions_->ReserveDelete(id);
		}
	private:
		void CreateThread() noexcept {
			threads_.reserve(thread::kNumWorkers + 1);
			for (int i = 0; i < thread::kNumWorkers; ++i) {
				threads_.emplace_back([this, i]() { WorkerThread(i); });
			};
			threads_.emplace_back(TimerThread);

			for (std::thread& th : threads_) {
				th.join();
			}
		}

		void WorkerThread(int id) noexcept;
		void ProcessAccept() noexcept;
		void Send() noexcept {
			//int cnt{};
			for (int i = thread::ID(); i < client::GetMaxClients(); i += thread::kNumWorkers) {
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
		void RunAI(float time) noexcept;

		static constexpr unsigned short kPort{ 21155 };
		static constexpr auto kTransferFrequency{ 1.0 / 5555555 };

		using SessionArray = lf::Array<client::Session>;
		std::vector<std::thread> threads_;
		sockaddr_in server_addr_;
		HANDLE iocp_;
		std::shared_ptr<Accepter> accepter_;
		std::shared_ptr<SessionArray> sessions_;
	};

	extern Socket sock;
};
