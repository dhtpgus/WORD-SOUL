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
				wsabuf_{}, rq_{ thread::GetNumWorker()} {
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
			WSARecv(sock_, &wsabuf_, 1, &recv_bytes_, &flags, &overlapped_, NULL);
		}

		template<class Packet>
		void Push(const Packet& p) {
			long long data = reinterpret_cast<long long>(&p) | (sizeof(Packet) << 48);

			rq_.Push(reinterpret_cast<packet::Base*>(data));
		}

		// thread safe 하지 않다.
		void Send() {
			memset(buf_, 0, GetBufferSize());
			send_bytes_ = 0;

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

				long long bin = reinterpret_cast<long long>(pop_value);
				WORD size = bin >> 48;
				packet::Base* p = reinterpret_cast<packet::Base*>(bin & 0x0000'FFFF'FFFF'FFFF);
				
				if (send_bytes_ + size > GetBufferSize()) {
					last_packet_ = pop_value;
					break;
				}

				memcpy(&buf_[send_bytes_ + sizeof(size)], p, size);

				delete p;

				DWORD* packet_head = (DWORD*)buf_;
				*packet_head += size;
				send_bytes_ += size;
			}

			if (send_bytes_ == 0) {
				return;
			}

			WSASend(sock_, &wsabuf_, 1, &send_bytes_, 0, &overlapped_, NULL);
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