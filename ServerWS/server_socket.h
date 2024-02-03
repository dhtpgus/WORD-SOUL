//---------------------------------------------------
// 
// server_socket.h - server::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <unordered_set>
#include <vector>
#include "client_socket.h"
#include "worker.h"

namespace server {
	constexpr unsigned short GetPortNum() {
		return 21155;
	}

	constexpr size_t GetBufferSize() {
		return 1024;
	}

	class Socket {
	public:
		Socket() : workers_{}, clients_{}, mx_{}, rq{ thread::GetNumWorker() } {
			if (WSAStartup(MAKEWORD(2, 2), &wsa_) != 0) {
				exit(1);
			}
			iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

			workers_.reserve(thread::GetNumWorker());

			for (int i = 0; i < thread::GetNumWorker(); ++i) {
				workers_.emplace_back(thread::Worker, iocp_, i, &rq);
			}

			for (std::thread& worker : workers_) {
				worker.detach();
			}

			listen_sock_ = socket(AF_INET, SOCK_STREAM, 0);
			if (listen_sock_ == INVALID_SOCKET) {
				exit(1);
			}

			memset(&server_addr_, 0, sizeof(server_addr_));
			server_addr_.sin_family = AF_INET;
			server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
			server_addr_.sin_port = htons(GetPortNum());
		}
		Socket(const Socket&) = delete;
		Socket(Socket&&) = delete;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = delete;
		~Socket() {
			WSACleanup();
		}

		void Bind() {
			if (bind(listen_sock_, (sockaddr*)&server_addr_, sizeof(server_addr_))
				== SOCKET_ERROR) {
				exit(1);
			}
		}
		void Listen() {
			if (listen(listen_sock_, SOMAXCONN) == SOCKET_ERROR) {
				exit(1);
			}
		}

		void Accept() {
			int sockaddr_len = sizeof(sockaddr_in);
			sockaddr_in client_sockaddr;
			SOCKET client_sock = accept(listen_sock_, (sockaddr*)&client_sockaddr, &sockaddr_len);
			if (client_sock == INVALID_SOCKET) {
				return;
			}

			mx_.lock();
			clients_.insert(new client::Socket{ client_sock, iocp_ });
			mx_.unlock();
		}
		void Disconnect(client::Socket* client_ptr) {
			mx_.lock();
			clients_.erase(client_ptr);
			delete client_ptr;
			mx_.unlock();
		}
	private:
		WSADATA wsa_;
		std::vector<std::thread> workers_;
		sockaddr_in server_addr_;
		SOCKET listen_sock_;
		HANDLE iocp_;
		std::unordered_set<client::Socket*> clients_;
		std::mutex mx_;
		lf::RelaxedQueue<int> rq;
	};

	server::Socket sock;
};