#include "server_socket.h"
#include "timer.h"

namespace server {
	Socket sock;
}

void server::Socket::AccepterThread(int thread_id)
{
	thread::ID(thread_id);

	clients_ = std::make_shared<ClientArray>(GetMaxClients(), thread::GetNumWorker() + 1);
	entity_manager_ = std::make_shared<entity::Manager>(GetMaxClients(), thread::GetNumWorker() + 1);

	std::print("[알림] accept 시작\n");

	int sockaddr_len = sizeof(sockaddr_in);
	while (true) {
		sockaddr_in client_sockaddr;
		SOCKET client_sock = accept(listen_sock_, (sockaddr*)&client_sockaddr, &sockaddr_len);
		if (client_sock == INVALID_SOCKET) {
			continue;
		}

		auto sock_id = clients_->Allocate<client::Socket>(client_sock, iocp_);
		if (sock_id == ClientArray::kInvalidID) {
			closesocket(client_sock);
			continue;
		}
		
		auto player_id = entity_manager_->AllocatePlayer();
		if (clients_->TryAccess(sock_id)) {
			(*clients_)[sock_id].SetPlayerID(player_id);
			clients_->EndAccess(sock_id);
		}
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
			if (clients_->Exists(s)) {
				std::print("ID: {} has disconnected.\n", s);
				clients_->ReserveDelete(s);
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
				id, clients_->Exists(id), transferred, client_ptr->GetBuffer());

			client_ptr->Push<packet::Position>(2, 4.0f, 5.0f, 6.0f);

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