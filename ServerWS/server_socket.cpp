#include "server_socket.h"
#include "debug.h"
#include "timer.h"

namespace server {
	Socket sock;
	constexpr auto kTransferFrequency{ 45.0 };
}

void server::Socket::ProcessAccept()
{
	static int addr_len = sizeof(sockaddr_in);
	auto client_sock = accept_sock_;
	accept_sock_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	AcceptEx(listen_sock_, accept_sock_, accept_over_.buf, 0,
		addr_len + 16, addr_len + 16, 0, &accept_over_.over);

	auto session_id = clients_->Allocate<client::Session>(client_sock, iocp_);
	if (session_id == ClientArray::kInvalidID) {
		closesocket(client_sock);
		return;
	}

	if (clients_->TryAccess(session_id)) {
		(*clients_)[session_id].Receive();
		(*clients_)[session_id].Push<packet::SCNewEntity>(session_id, 0.0f, 0.0f, 0.0f, entity::Type::kPlayer);
		clients_->EndAccess(session_id);
	}
}

void server::Socket::WorkerThread(int thread_id)
{
	thread::ID(thread_id);

	DWORD transferred{};
	ULONG_PTR key;
	OverEx* over_ex{};
	int retval{};

	Timer timer;

	while (true) {
		retval = GetQueuedCompletionStatus(iocp_, &transferred, &key,
			reinterpret_cast<LPOVERLAPPED*>(&over_ex), 1);

		int id = static_cast<int>(key);
		auto duration{ timer.GetDuration() };

		if (0 == retval) {
		}
		else if (over_ex->op == Operation::kAccept) {
			ProcessAccept();
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
					if (false == (*clients_)[i].Send()) {
						Disconnect(i);
					}
					clients_->EndAccess(i);
				}
			}
		}
	}
}