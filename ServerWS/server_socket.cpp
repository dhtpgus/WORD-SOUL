#include "server_socket.h"
#include "free_list.h"
#include "debug.h"
#include "timer.h"
#include "monster.h"

namespace server {
	Socket sock;

	void Socket::ProcessAccept() noexcept
	{
		auto client_sock = accepter_->GetAcceptedSocket();
		accepter_->Accept();

		auto session_id = sessions_->Allocate<client::Session>(client_sock, iocp_);
		if (session_id == SessionArray::kInvalidID) {
			closesocket(client_sock);
			return;
		}

		if (sessions_->TryAccess(session_id)) {
			auto& session = (*sessions_)[session_id];
			session.Receive();
			session.Emplace<packet::SCNewEntity>(entity::kAvatarID, 80.0f, 500.0f, 0.0f, entity::Type::kPlayer, 0);
			session.GetPlayer().SetPosition(80.0f, 500.0f, 0.0f);
			sessions_->EndAccess(session_id);
		}
	}

	void Socket::WorkerThread(int thread_id) noexcept
	{
		thread::ID(thread_id);

		DWORD transferred{};
		ULONG_PTR key{};
		OverEx* ox{};
		int retval{};

		Timer timer;
		Timer::Duration duration{};
		Timer::Duration ac_duration{};
		Timer::Duration ac_duration2{};

		const DWORD kDelay = debug::DisplaysMSG() ? 10UL : 0UL;

		while (true) {
			key = 0xFFFF'FFFF'FFFF'FFFF;
			retval = GetQueuedCompletionStatus(iocp_, &transferred, &key,
				reinterpret_cast<LPOVERLAPPED*>(&ox), kDelay);

			int id = static_cast<int>(key);

			if (0 == retval) {
				if (sessions_->TryAccess(id)) {
					Disconnect(id);
					sessions_->EndAccess(id);
				}
			}
			else {
				switch (ox->op)
				{
				case Operation::kSend:
					free_list<OverEx>.Collect(ox);
					break;
				case Operation::kAccept:
					ProcessAccept();
					break;
				case Operation::kRecv:
					if (transferred == 0) {
						break;
					}
					if (sessions_->TryAccess(id)) {
						auto& buffer = (*sessions_)[id].GetBuffer();
						transferred += buffer.GetSizeRemains();

						/*if (debug::DisplaysMSG()) {
							std::print("[MSG] {}({}): {}\n", id, sessions_->Exists(id),
								buffer.GetBinary(transferred));
						}*/

						while (transferred != 0) {
							ProcessPacket(buffer, transferred, id);
						}
						(*sessions_)[id].Receive();

						sessions_->EndAccess(id);
					}
					break;
				}
			}

			duration = timer.GetDuration();
			ac_duration += duration;

			RunAI(ac_duration);

			if (ac_duration >= kTransferFrequency) {
				//if (rng.Rand(0, 10) == 0) std::print("{}: {}\n", thread::ID(), ac_duration);
				ac_duration = 0.0;

				Send();
			}
		}
	}

	void Socket::ProcessPacket(BufferRecv& buf, DWORD& n_bytes, int session_id) noexcept
	{
		//std::print("{}: {}\n", thread::ID(), n_bytes);

		auto size = buf.GetPacketSize();
		if (n_bytes < sizeof(size) + sizeof(packet::Type) + size) {
			buf.SaveRemains(n_bytes);
			n_bytes = 0;
			return;
		}

		auto type = buf.GetPacketType();
		n_bytes -= sizeof(size) + sizeof(type) + size;

		auto& session = (*sessions_)[session_id];

		switch (type) {
		case packet::Type::kTest:
		{
			packet::Test p{ buf.GetData() };
			std::print("(ID: {}) test: {} {} {}, {}\n", p.a, p.b, p.c, n_bytes);
			break;
		}
		case packet::Type::kCSJoinParty:
		{
			packet::CSJoinParty p{ buf.GetData() };
			if (parties[p.id].TryEnter(session_id)) {
				if (debug::DisplaysMSG()) {
					std::print("(ID: {}) has joined Party: {}.\n", session_id, p.id);
				}

				session.Emplace<packet::SCResult>(true);
				session.SetPartyID(p.id);

				auto partner_id = parties[p.id].GetPartnerID(session_id);
				
				if (sessions_->TryAccess(partner_id)) {
					auto& pos = session.GetPlayer().GetPostion();
					(*sessions_)[partner_id].Emplace<packet::SCNewEntity>(
						entity::kPartnerID, pos.x, pos.y, pos.z, entity::Type::kPlayer, 0);
					sessions_->EndAccess(partner_id);
				}
			}
			else {
				session.Emplace<packet::SCResult>(false);
			}
			break;
		}
		case packet::Type::kCSPosition:
		{
			packet::CSPosition p{ buf.GetData() };

			if (debug::DisplaysMSG()) {
				//std::print("(ID: {}) (x, y, z) = ({}, {}, {})\n", session_id, p.x, p.y, p.z);
			}
			
			session.GetPlayer().SetPosition(p.x, p.y, p.z);
			auto party_id{ session.GetPartyID() };
			auto partner_id = parties[party_id].GetPartnerID(session_id);
			
			if (sessions_->TryAccess(partner_id)) {
				(*sessions_)[partner_id].Emplace<packet::SCPosition>(entity::kPartnerID, p.x, p.y, p.z, p.v, p.flag);
				sessions_->EndAccess(partner_id);
			}
			break;
		}
		case packet::Type::kCSLeaveParty:
		{
			auto party_id = session.GetPartyID();
			if (party_id < 0 or party_id >= parties.size()) {
				break;
			}
			parties[party_id].Exit(session_id);
			break;
		}
		default:
		{
			std::print("[Error] Unknown Packet: {}\n", static_cast<int>(type));
			system("pause");
			exit(1);
		}
		}

		if (n_bytes == 0) {
			buf.ResetCursor();
		}
		else {
			buf.MoveCursor(sizeof(size) + sizeof(type) + size);
		}
	}

	void Socket::RunAI(float time) noexcept
	{
		for (int i = thread::ID(); i < parties.size(); i += thread::GetNumWorkers()) {
			if (not parties[i].IsAssembled()) {
				continue;
			}
			auto members{ parties[i].GetPartyMembers() };
			const Position* pos[2]{};
			bool break_flag{};

			for (int p = 0; p < 2; ++p) {
				if (sessions_->TryAccess(members[p])) {
					pos[p] = &(*sessions_)[members[p]].GetPlayer().GetPostion();
					sessions_->EndAccess(members[p]);
				}
				else {
					break_flag = true;
					break;
				}
			}
			if (true == break_flag) {
				continue;
			}

			entity::managers[i].Update(*pos[0], *pos[1], time);

			for (int j = 0; j < entity::kMaxEntities; ++j) {
				if (entity::managers[i].TryAccess(j)) {
					auto& en = entity::managers[i][j];
					auto& en_pos = en.GetPostion();

					for (auto mem : members) {
						if (sessions_->TryAccess(mem)) {
							(*sessions_)[mem].Emplace<packet::SCPosition>(en.GetID(), en_pos.x, en_pos.y, en_pos.z, en.GetVel(), en.GetFlag());
							
							//std::print("{}: {} {} {}\n", en.GetID(), en_pos.x, en_pos.y, en_pos.z);
							sessions_->EndAccess(mem);
						}
					}

					entity::managers[i].EndAccess(j);
				}
			}
		}
	}
}

