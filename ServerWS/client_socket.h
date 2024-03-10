//---------------------------------------------------
// 
// client_socket.h - client::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>
#include <mutex>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "packet.h"
#include "lf_relaxed_queue.h"
#include "debug.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

namespace client {
	class Socket {
	public:
		Socket() = delete;
		Socket(int id, SOCKET sock, HANDLE iocp)
			: overlapped_{}, sock_{ sock }, buf_{}, recv_bytes_{}, send_bytes_{},
				wsabuf_{}, id_{ id }, player_id_{}, rq_{ thread::GetNumWorker() } {
			wsabuf_.buf = buf_;
			wsabuf_.len = (ULONG)kBufferSize;
			CreateIoCompletionPort((HANDLE)sock_, iocp, sock_, 0);
			StartAsyncIO();
			if (debug::IsDebugMode()) {
				std::print("[Info] ID: {} has joined.\n", GetID());
			}
		}
		~Socket() {
			if (debug::IsDebugMode()) {
				std::print("[Info] ID: {} has left.\n", GetID());
			}
			closesocket(sock_);
		}
		Socket(const Socket&) = delete;
		Socket(Socket&&) = default;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = default;

		void StartAsyncIO() {
			DWORD flags = 0;
			wsabuf_.buf = buf_;
			WSARecv(sock_, &wsabuf_, 1, &recv_bytes_, &flags, &overlapped_, nullptr);
		}

		template<class Packet, class... Value>
		void Push(Value... value) {
			rq_.Emplace<Packet>(value...);
		}

		// WSASend 함수로 재작성 필요
		void Send() {
			char buf_2[kBufferSize]{};
			//memset(buf_, 0, GetBufferSize());

			DWORD send_bytes = 0;

			while (true) {
				packet::Base* pop_value{};

				if (last_packet_ == nullptr) {
					pop_value = rq_.Pop();

					if (pop_value == lf::kPopFailed) {
						last_packet_ = nullptr;
						break;
					}
				}
				else {
					pop_value = last_packet_;
				}

				size_t size = pop_value->size;
				
				if (send_bytes + size > kBufferSize) {
					last_packet_ = pop_value;
					break;
				}

				buf_2[send_bytes] = (unsigned char)(pop_value->type);
				memcpy(&buf_2[send_bytes + 1], ((char*)pop_value) + sizeof(packet::Base), size);

				send_bytes += 1 + (DWORD)size;

				delete pop_value;
			}

			if (send_bytes == 0) {
				return;
			}

			if (debug::IsDebugMode()) {
				std::print("[Info] {}\n", packet::CheckBytes(buf_2, send_bytes));
			}

			send(sock_, buf_2, send_bytes, 0);
			//WSASend(sock_, &wsabuf_2, 1, &send_bytes, 0, &overlapped_, nullptr);
		}

		const char* GetBuffer() const {
			return buf_;
		}

		int GetID() const {
			return abs(id_);
		}

		SOCKET GetSocket() const {
			return sock_;
		}

		int GetPlayerID(int id) const {
			return player_id_;
		}

		void SetPlayerID(int id) {
			player_id_ = id;
		}

		bool IsLogicallyDeleted() const {
			return id_ < 0;
		}
		void DeleteLogically() {
			id_ *= -1;
		}
		static constexpr bool is_dangerous_to_delete{ true };

	private:
		static constexpr size_t kBufferSize = 1024;

		OVERLAPPED overlapped_;
		SOCKET sock_;
		char buf_[kBufferSize];
		DWORD recv_bytes_;
		DWORD send_bytes_;
		WSABUF wsabuf_;
		int id_;
		int player_id_;
		lf::RelaxedQueue<packet::Base> rq_;
		packet::Base* last_packet_;
	};
}