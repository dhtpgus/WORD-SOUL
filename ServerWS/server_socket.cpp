#include "server_socket.h"
#include "free_list.h"
#include "debug.h"
#include "timer.h"

namespace server {
	Socket sock;
}

void server::Socket::ProcessAccept() noexcept
{
	auto client_sock = accepter_->GetAcceptedSocket();
	accepter_->Accept();

	auto session_id = clients_->Allocate<client::Session>(client_sock, iocp_);
	if (session_id == ClientArray::kInvalidID) {
		closesocket(client_sock);
		return;
	}

	if (clients_->TryAccess(session_id)) {
		(*clients_)[session_id].Receive();
		(*clients_)[session_id].Push<packet::SCNewEntity>(
			entity::kAvatarID, 0.0f, 0.0f, 0.0f, entity::Type::kPlayer);
		clients_->EndAccess(session_id);
	}
}

void server::Socket::WorkerThread(int thread_id) noexcept
{
	thread::ID(thread_id);

	DWORD transferred{};
	ULONG_PTR key;
	OverEx* ox{};
	int retval{};

	Timer timer;

	while (true) {
		retval = GetQueuedCompletionStatus(iocp_, &transferred, &key,
			reinterpret_cast<LPOVERLAPPED*>(&ox), 1);

		int id = static_cast<int>(key);
		auto duration{ timer.GetDuration() };

		if (0 == retval) {
		}
		else if (ox->op == Operation::kSend) {
			free_list<OverEx>.Collect(ox);
		}
		else if (ox->op == Operation::kAccept) {
			ProcessAccept();
		}
		else if (transferred != 0) {
			if (clients_->TryAccess(id)) {
				if (debug::IsDebugMode()) {
					std::print("[MSG] {}({}): {}\n", id, clients_->Exists(id),
						packet::CheckBytes(const_cast<char*>((*clients_)[id].GetBuffer()), transferred));
				}
				
				char* buffer = const_cast<char*>((*clients_)[id].GetBuffer());

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