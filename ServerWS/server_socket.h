//---------------------------------------------------
// 
// server_socket.h - server::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <vector>
#include "client_socket.h"
#include "lf_array.h"

namespace server {
	class Socket {
	public:
		Socket() : threads_{}, clients_{ 1000, thread::GetNumWorker() + 1 } {
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
			server_addr_.sin_port = htons(kPortNum);
		}
		Socket(const Socket&) = delete;
		Socket(Socket&&) = delete;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = delete;
		~Socket() {
			WSACleanup();
		}

		void Start() {
			Bind();
			Listen();
			CreateThread();
		}

		void Disconnect(client::Socket::ID id) {
			clients_.ReserveDelete(id);
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
			threads_.emplace_back([this]() { AccepterThread(thread::GetNumWorker()); });

			for (std::thread& th : threads_) {
				th.join();
			}
		}

		void AccepterThread(int id);
		void WorkerThread(int id);

		static constexpr unsigned short kPortNum = 21155;
		static constexpr size_t kBufferSize = 1024;

		WSADATA wsa_;
		std::vector<std::thread> threads_;
		sockaddr_in server_addr_;
		SOCKET listen_sock_;
		HANDLE iocp_;
		lf::Array<client::Socket> clients_;
	};

	extern Socket sock;
};
