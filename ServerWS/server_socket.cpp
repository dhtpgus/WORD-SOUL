#include "server_socket.h"

namespace server {
	Socket sock;
}

void server::Socket::AccepterThread()
{
	int sockaddr_len = sizeof(sockaddr_in);
	while (true) {
		sockaddr_in client_sockaddr;
		SOCKET client_sock = accept(listen_sock_, (sockaddr*)&client_sockaddr, &sockaddr_len);
		if (client_sock == INVALID_SOCKET) {
			continue;
		}

		clients_.Add(new client::Socket{ client_sock, iocp_ });
	}
}

void server::Socket::WorkerThread(int thread_id)
{
	DWORD transferred;
	SOCKET client_sock;
	client::Socket* client_ptr;
	int retval;

	// ÀÓ½Ã·Î ³öµÒ
	// auto tp = std::chrono::steady_clock::now();
	// std::chrono::nanoseconds d{};

	while (true) {
		retval = GetQueuedCompletionStatus(iocp_, &transferred, &client_sock,
			(LPOVERLAPPED*)&client_ptr, 1);

		if (retval == 0) {
			//continue;
		}
		else if (transferred != 0) {
			std::print("{}: [{}] {}\n", client_sock, transferred, client_ptr->GetBuffer());
			
			client_ptr->Push(new packet::Position{ 0, 4, 5, 6 });

			client_ptr->Send();

			client_ptr->StartAsyncIO();
		}
	}
}