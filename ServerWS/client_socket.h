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
#pragma comment(lib, "ws2_32")

namespace client {
	constexpr size_t GetBufferSize() {
		return 1024;
	}

	class Socket {
	public:
		Socket() = default;
		Socket(SOCKET sock, HANDLE iocp)
			: overlapped_{}, sock_{ sock }, buf_{}, recv_bytes_{}, send_bytes_{}, wsabuf_{} {
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

		void Send() {
			DWORD send_bytes;
			WSASend(sock_, &wsabuf_, 1, &send_bytes, 0, &overlapped_, NULL);
		}

		const char* GetBuffer() const {
			return buf_;
		}
	private:
		OVERLAPPED overlapped_{};
		SOCKET sock_{};
		char buf_[GetBufferSize()]{};
		DWORD recv_bytes_{};
		DWORD send_bytes_{};
		WSABUF wsabuf_{};
	};
}