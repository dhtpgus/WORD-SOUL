#pragma once
#include "session.h"

namespace server {
	class Accepter {
	public:
		Accepter() noexcept
			: ox_{ Operation::kAccept }, buf_{},
			listen_sock_{ WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED) },
			accept_sock_{ WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED) } {
		}
		~Accepter() noexcept {
			closesocket(accept_sock_);
			closesocket(listen_sock_);
		}
		void Accept() noexcept {
			accept_sock_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			AcceptEx(listen_sock_, accept_sock_, buf_.data(), 0,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 0, &ox_.over);
		}
		void LinkIOCP(HANDLE iocp) noexcept {
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_sock_), iocp, 9999, 0);
		}
		auto GetAcceptedSocket() const noexcept {
			return accept_sock_;
		}
		void BindAndListen(sockaddr_in& server_addr) noexcept {
			bind(listen_sock_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
			listen(listen_sock_, SOMAXCONN);
		}
	private:
		OverEx ox_;
		Buffer buf_;
		SOCKET listen_sock_;
		SOCKET accept_sock_;
	};
}