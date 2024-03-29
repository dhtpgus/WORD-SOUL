#include "server_socket.h"
#include "debug.h"
#include "timer.h"

namespace server {
	Socket sock;
	constexpr auto kTransferFrequency{ 45.0 };
}

void server::Socket::AccepterThread(int thread_id)
{
	thread::ID(thread_id);

	if (debug::IsDebugMode()) {
		std::print("[Info] Started to Accept\n");
	}

	int sockaddr_len = sizeof(sockaddr_in);
	while (true) {
		sockaddr_in client_sockaddr;

		SOCKET client_sock = WSAAccept(listen_sock_, (sockaddr*)&client_sockaddr, &sockaddr_len, 0, 0);
		if (client_sock == INVALID_SOCKET) {
			continue;
		}

		auto session_id = clients_->Allocate<client::Session>(client_sock, iocp_);
		if (session_id == ClientArray::kInvalidID) {
			closesocket(client_sock);
			continue;
		}

		if (clients_->TryAccess(session_id)) {
			(*clients_)[session_id].Receive();
			(*clients_)[session_id].Push<packet::SCNewEntity>(session_id, 0.0f, 0.0f, 0.0f, entity::Type::kPlayer);
			clients_->EndAccess(session_id);
		}
	}
}

void server::Socket::WorkerThread(int thread_id)
{
	thread::ID(thread_id);

	DWORD transferred{};
	ULONG_PTR key;
	client::Session* client_ptr{};
	int retval{};

	Timer timer;

	while (true) {
		retval = GetQueuedCompletionStatus(iocp_, &transferred, &key,
			(LPOVERLAPPED*)&client_ptr, 1);

		int id = static_cast<int>(key);
		auto duration{ timer.GetDuration() };

		if (0 == retval) {
		}
		else if (transferred != 0) {
			if (clients_->TryAccess(id)) {
				if (debug::IsDebugMode()) {
					std::print("[MSG] {}({}): {}\n", id, clients_->Exists(id),
						packet::CheckBytes((*clients_)[id].GetBuffer(), transferred));
				}
				
				char* buffer = (char*)((*clients_)[id].GetBuffer());

				while (transferred != 0) {
					Deserialize(buffer, transferred, id);
				}
				(*clients_)[id].Receive();

				clients_->EndAccess(id);
			}
		}

		auto c = timer.GetAccumulatedDuration();
		if (c >= 1.0 / kTransferFrequency) {
			timer.ResetAccumulatedDuration();

			for (int i = thread::ID(); i < GetMaxClients(); i += thread::GetNumWorker()) {
				if (clients_->TryAccess(i)) {
					int ret = (*clients_)[i].Send();
					if (ret != 0) {
						clients_->ReserveDelete(i);
					}
					clients_->EndAccess(i);
				}
			}
		}
	}
}