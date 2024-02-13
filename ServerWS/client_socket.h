//---------------------------------------------------
// 
// client_socket.h - client::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <print>
#include "packet.h"
#include "lf_relaxed_queue.h"
#pragma comment(lib, "ws2_32")

namespace client {
	constexpr size_t GetBufferSize() {
		return 1024;
	}

	class Socket {
	public:
		Socket() = delete;
		Socket(SOCKET sock, HANDLE iocp)
			: overlapped_{}, sock_{ sock }, buf_{}, recv_bytes_{}, send_bytes_{},
				wsabuf_{}, rq_{ thread::GetNumWorker() } {
			wsabuf_.buf = buf_;
			wsabuf_.len = (ULONG)GetBufferSize();

			CreateIoCompletionPort((HANDLE)sock_, iocp, sock_, 0);
			StartAsyncIO();
			std::cout << std::format("{}\n", (long long)this);
		}
		~Socket() {
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

		void Push(packet::Base* p) {
			rq_.Push(p);
		}

		void Send() {
			char buf_2[GetBufferSize()]{};
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
				
				if (send_bytes + size > GetBufferSize()) {
					last_packet_ = pop_value;
					break;
				}

				buf_2[send_bytes] = (unsigned char)pop_value->type;
				memcpy(&buf_2[send_bytes + 1], ((char*)pop_value) + 4, size);

				send_bytes += 1 + (DWORD)size;

				delete pop_value;
			}

			if (send_bytes == 0) {
				return;
			}

			send(sock_, buf_2, send_bytes, 0);
			//WSASend(sock_, &wsabuf_2, 1, &send_bytes, 0, &overlapped_, nullptr);
		}

		const char* GetBuffer() const {
			return buf_;
		}
	private:
		OVERLAPPED overlapped_;
		SOCKET sock_;
		char buf_[GetBufferSize()];
		DWORD recv_bytes_;
		DWORD send_bytes_;
		WSABUF wsabuf_;
		lf::RelaxedQueue<packet::Base> rq_;
		packet::Base* last_packet_;
	};
}