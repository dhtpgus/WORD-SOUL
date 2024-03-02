#include "server_socket.h"
#include "timer.h"

namespace server {
	Socket sock;
}

void server::Socket::AccepterThread(int thread_id)
{
	thread::ID(thread_id);

	clients_.InitIndexes(GetMaxClients());

	int sockaddr_len = sizeof(sockaddr_in);
	while (true) {
		sockaddr_in client_sockaddr;
		SOCKET client_sock = accept(listen_sock_, (sockaddr*)&client_sockaddr, &sockaddr_len);
		if (client_sock == INVALID_SOCKET) {
			continue;
		}

		clients_.Allocate(client_sock, iocp_);
	}
}

void server::Socket::WorkerThread(int thread_id)
{
	thread::ID(thread_id);

	DWORD transferred{};
	SOCKET client_sock{};
	client::Socket* client_ptr{};
	int retval{};

	Timer timer;

	while (true) {
		if (thread::ID() == 0) {
			int s;
			std::cin >> s;
			if (clients_.Exists(s)) {
				std::print("{} exists\n", s);
				clients_.ReserveDelete(s);
			}
			continue;
		}

		retval = GetQueuedCompletionStatus(iocp_, &transferred, &client_sock,
			(LPOVERLAPPED*)&client_ptr, 1);

		auto duration{ timer.GetDuration() };

		if (retval == 0) {
			//continue;
		}
		else if (transferred != 0) {
			auto id = client_ptr->GetID();
			std::print("{}({}): [{}] {}\n",
				id, clients_.Exists(id), transferred, client_ptr->GetBuffer());

			client_ptr->Push<packet::Position>(0, 4.0f, 5.0f, 6.0f);

			client_ptr->Send();

			client_ptr->StartAsyncIO();
		}

		auto c = timer.GetAccumulatedDuration();
		if (c >= 1.0 / 45) {
			//...
			timer.ResetAccumulatedDuration();
		}
	}
}