#pragma once
#include "server_socket.h"
#include "free_list.h"
#include "debug.h"
#include "timer.h"

auto GetPartyID2(int id) noexcept
{
	return static_cast<Party::ID>(id >> 16);
}

namespace server {
	void Socket::WorkerThreadBlocking(int thread_id) noexcept
	{
		thread::ID(thread_id);

		DWORD transferred{};
		ULONG_PTR key{};
		OverEx* ox{};
		int retval{};

		Timer timer;
		Timer::Duration duration{};
		Timer::Duration ac_duration{};

		while (true) {
			key = 0xFFFF'FFFF'FFFF'FFFF;
			retval = GetQueuedCompletionStatus(iocp_, &transferred, &key,
				reinterpret_cast<LPOVERLAPPED*>(&ox), INFINITE);

			int id = static_cast<int>(key);

			if (0 == retval) {
				if (sessions.TryAccess(id)) {
					Disconnect(id);
					sessions.EndAccess(id);
				}
			}
			else {
				switch (ox->op)
				{
				case Operation::kSend:
				{
					free_list<OverEx>.Collect(ox);
					break;
				}
				case Operation::kAccept:
				{
					ProcessAccept();
					break;
				}
				case Operation::kRecv:
				{
					if (0 == transferred) {
						break;
					}
					if (sessions.TryAccess(id)) {
						auto& buffer = sessions[id].GetBuffer();
						transferred += buffer.GetSizeRemains();

						while (transferred != 0) {
							ProcessPacket(buffer, transferred, id);
						}
						sessions[id].Receive();
						sessions.EndAccess(id);
					}
					break;
				}

				case Operation::kUpdateMobAI:
				{
					if (entity::managers[GetPartyID2(id)].UpdateAI(id & 0xFFFF)) {
						timer::event_pq->Emplace(id, 20, Operation::kUpdateMobAI);
					}
					free_list<OverEx>.Collect(ox);
					break;
				}
				}
			}
		}
	}
}