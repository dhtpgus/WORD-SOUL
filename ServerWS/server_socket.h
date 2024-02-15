//---------------------------------------------------
// 
// server_socket.h - server::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <unordered_set>
#include <vector>
#include "client_socket.h"

namespace server {
	constexpr unsigned short GetPortNum() {
		return 21155;
	}

	constexpr size_t GetBufferSize() {
		return 1024;
	}

	class Socket {
	public:
		Socket() : threads_{}, clients_{}, mx_{}, rq{ thread::GetNumWorker() } {
			if (WSAStartup(MAKEWORD(2, 2), &wsa_) != 0) {
				exit(1);
			}
			iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

			listen_sock_ = socket(AF_INET, SOCK_STREAM, 0);
			if (listen_sock_ == INVALID_SOCKET) {
				exit(1);
			}

			memset(&server_addr_, 0, sizeof(server_addr_));
			server_addr_.sin_family = AF_INET;
			server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
			server_addr_.sin_port = htons(GetPortNum());

			Bind();
			Listen();
			CreateThread();
		}
		Socket(const Socket&) = delete;
		Socket(Socket&&) = delete;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = delete;
		~Socket() {
			WSACleanup();
		}

		void Disconnect(client::Socket* client_ptr) {
			mx_.lock();
			clients_.erase(client_ptr);
			delete client_ptr;
			mx_.unlock();
		}
	private:
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
			}
			threads_.emplace_back([this]() { AccepterThread(); });

			for (std::thread& th : threads_) {
				th.join();
			}
		}
		void AccepterThread();
		void WorkerThread(int id);

		WSADATA wsa_;
		std::vector<std::thread> threads_;
		sockaddr_in server_addr_;
		SOCKET listen_sock_;
		HANDLE iocp_;
		std::unordered_set<client::Socket*> clients_;
		std::mutex mx_;
		lf::RelaxedQueue<int> rq;
	};

	extern Socket sock;
};
