#include "server_socket.h"
#include "free_list.h"
#include "debug.h"
#include "timer.h"

auto GetPartyID(int id) noexcept
{
	return static_cast<Party::ID>(id >> 16);
}

namespace server {
	Socket sock;

	//inline int ccnt;

	void Socket::ProcessAccept() noexcept
	{
		//std::print("c cnt: {}\n", ++ccnt);

		auto client_sock = accepter_->GetAcceptedSocket();
		accepter_->Accept();

		auto session_id = sessions.Allocate<client::Session>(client_sock, iocp_);
		if (session_id == SessionArray::kInvalidID) {
			closesocket(client_sock);
			return;
		}

		if (sessions.TryAccess(session_id)) {
			auto& session = sessions[session_id];
			session.Receive();
			session.Emplace<packet::SCNewEntity>(entity::kAvatarID, 80.0f, 500.0f, 0.0f, entity::Type::kPlayer, 0);
			session.GetPlayer().SetPosition(80.0f, 500.0f, 0.0f);
			sessions.EndAccess(session_id);
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
#if USES_NONBLOCKING_DS
					if (sessions.TryAccess(id)) {
#endif
						auto& buffer = sessions[id].GetBuffer();
						transferred += buffer.GetSizeRemains();

						while (transferred != 0) {
							ProcessPacket(buffer, transferred, id);
						}
						sessions[id].Receive();
#if USES_NONBLOCKING_DS
						sessions.EndAccess(id);
					}
#endif
					break;
				}

				case Operation::kUpdateMobAI:
				{
					if (entity::managers[GetPartyID(id)].UpdateAI(id % 0x10000)) {
						timer::event_pq->Emplace(id, 20, Operation::kUpdateMobAI);
					}
					free_list<OverEx>.Collect(ox);
					break;
				}
				}
			}
#if USES_NONBLOCKING_DS
			//duration = timer.GetDuration();
			//ac_duration += duration;

			//if (ac_duration >= kTransferCheckFrequency) {
			//	//if (rng.Rand(0, 10) == 0) std::print("{}: {}\n", thread::ID(), ac_duration);
			//	ac_duration = 0.0;

			//	Send();
			//}
#endif
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

		auto& session = sessions[session_id];

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
			//std::print("(ID: {}) tried to join Party: {}.\n", session_id, p.id);
			if (parties[p.id].TryEnter(session_id)) {
				if (debug::DisplaysMSG()) {
					std::print("(ID: {}) has joined Party: {}.\n", session_id, p.id);
				}

				session.Emplace<packet::SCResult>(true);
				session.SetPartyID(p.id);

				auto partner_id = parties[p.id].GetPartnerID(session_id);
				
				if (sessions.TryAccess(partner_id)) {
					auto& pos = session.GetPlayer().GetPosition();
					sessions[partner_id].Emplace<packet::SCNewEntity>(
						entity::kPartnerID, pos.x, pos.y, pos.z, entity::Type::kPlayer, 0);
					sessions.EndAccess(partner_id);
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
				std::print("(ID: {}) (x, y, z) = ({}, {}, {})\n", session_id, p.x, p.y, p.z);
			}

			auto& player = session.GetPlayer();
			
			auto region = player.SetPosition(p.x, p.y, p.z);
			player.dir_ = p.r;
			player.flag_ = p.flag;
			auto party_id{ session.GetPartyID() };

			if (-1 == party_id) {
				break;
			}

			auto partner_id = parties[party_id].GetPartnerID(session_id);

			if (sessions.TryAccess(partner_id)) {
				sessions[partner_id].Emplace<packet::SCPosition>(entity::kPartnerID, p.x, p.y, p.z, p.v, p.r, p.flag);
				sessions.EndAccess(partner_id);
			}
			if (-1 == region) {
				break;
			}

			std::vector<int> en_ids;
			en_ids.reserve(30);

			auto& entities = entity::managers[party_id];
			entities.GetEntitiesInRegion(region, en_ids);

			for (entity::ID en_id : en_ids) {
				if (entities.TryAccess(en_id)) { 
					auto& en = entities[en_id];
					const auto& en_pos = en.GetPosition();

					if (entity::CanSee(en_pos, player.GetPosition())) {
						if (view_lists[session.GetID()]->Insert(en_id)) {
							session.Emplace<packet::SCNewEntity>(en_id,
								en_pos.x, en_pos.y, en_pos.z, en.GetType(), en.GetFlag());
						}
						
						session.Emplace<packet::SCPosition>(&en);
						if (entity::Type::kMob == en.GetType()) {
							reinterpret_cast<entity::Mob*>(&en)->WakeUp();
						}
					}
					else {
						session.Emplace<packet::SCRemoveEntity>(en_id, char{ 1 });
					}
					entities.EndAccess(en_id);
				}
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
		case packet::Type::kStressTest:
		{
			packet::StressTest p{ buf.GetData() };

			auto& player = session.GetPlayer();
			auto region = player.SetPosition(p.x, p.y, p.z);
			session.Emplace<packet::StressTest>(static_cast<entity::ID>(session_id), p.x, p.y, p.z, p.tp);

			auto party_id{ session.GetPartyID() };

			if (-1 == party_id) {
				break;
			}

			auto partner_id = parties[party_id].GetPartnerID(session_id);

			if (sessions.TryAccess(partner_id)) {
				//sessions[partner_id].Emplace<packet::SCPosition>(entity::kPartnerID, p.x, p.y, p.z, 0.0f, 0.0f, char{0});
				sessions.EndAccess(partner_id);
			}
			if (-1 == region) {
				break;
			}

			std::vector<int> en_ids;
			en_ids.reserve(30);

			auto& entities = entity::managers[party_id];
			entities.GetEntitiesInRegion(region, en_ids);

			for (entity::ID en_id : en_ids) {
				if (entities.TryAccess(en_id)) {
					auto& en = entities[en_id];
					const auto& en_pos = en.GetPosition();

					if (entity::CanSee(en_pos, player.GetPosition())) {
						if (view_lists[session.GetID()]->Insert(en_id)) {
							session.Emplace<packet::SCNewEntity>(en_id,
								en_pos.x, en_pos.y, en_pos.z, en.GetType(), en.GetFlag());
						}

						session.Emplace<packet::SCPosition>(&en);
						if (entity::Type::kMob == en.GetType()) {
							reinterpret_cast<entity::Mob*>(&en)->WakeUp();
						}
					}
					else {
						session.Emplace<packet::SCRemoveEntity>(en_id, char{ 1 });
					}
					entities.EndAccess(en_id);
				}
			}
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

	
}

