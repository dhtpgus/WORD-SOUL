#include "server_socket.h"

namespace server {
	Socket sock;
}

void server::Socket::Worker(int thread_id)
{
	DWORD transferred;
	SOCKET client_sock;
	client::Socket* client_ptr;
	int retval;

	auto tp = std::chrono::steady_clock::now();
	auto d = std::chrono::nanoseconds{};

	while (true) {
		retval = GetQueuedCompletionStatus(iocp_, &transferred, &client_sock,
			(LPOVERLAPPED*)&client_ptr, 1);

		if (retval == 0) {
			//continue;
		}
		else if (transferred != 0) {
			std::print("{}: [{}] {}\n", client_sock, transferred, client_ptr->GetBuffer());
			
			client_ptr->Push(new packet::Position{ 4, 5, 6 });

			client_ptr->Send();

			client_ptr->StartAsyncIO();
		}
	}
}