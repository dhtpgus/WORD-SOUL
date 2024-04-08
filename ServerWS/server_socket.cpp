#include "server_socket.h"
#include "free_list.h"
#include "debug.h"
#include "timer.h"

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
			(*sessions_)[session_id].Receive();
			(*sessions_)[session_id].Push<packet::SCNewEntity>(
				entity::kAvatarID, 0.0f, 0.0f, 0.0f, entity::Type::kPlayer);
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
			retval = GetQueuedCompletionStatus(iocp_, &transferred, &key,
				reinterpret_cast<LPOVERLAPPED*>(&ox), kDelay);

			int id = static_cast<int>(key);

			if (0 == retval) {
			}
			else if (ox->op == Operation::kSend) {
				free_list<OverEx>.Collect(ox);
			}
			else if (ox->op == Operation::kAccept) {
				ProcessAccept();
			}
			else if (transferred != 0) {
				if (sessions_->TryAccess(id)) {
					auto& buffer = (*sessions_)[id].GetBuffer();

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
			}

			duration = timer.GetDuration();
			ac_duration += duration;

			if (ac_duration >= kTransferFrequency) {
				//if (rng.Rand(0, 10) == 0) std::print("{}: {}\n", thread::ID(), ac_duration);
				ac_duration = 0.0;

				Send();
			}
		}
	}

	void Socket::ProcessPacket(BufferRecv& buf, DWORD& n_bytes, int session_id) noexcept
	{
		packet::Size size = *(packet::Size*)(buf.GetData());
		if (n_bytes < sizeof(size) + sizeof(packet::Type) + size) {
			buf.SaveRemains(n_bytes);
			n_bytes = 0;
			return;
		}

		packet::Type type = *(packet::Type*)(buf.GetData() + 1);
		n_bytes -= sizeof(size) + sizeof(type) + size;

		switch (type) {
		case packet::Type::kTest: {
			packet::Test p{ buf.GetData() };
			std::print("{} {} {}, {}\n", p.a, p.b, p.c, n_bytes);
			break;
		}
		case packet::Type::kCSJoinParty: {
			packet::CSJoinParty p{ buf.GetData() };
			if (parties_[p.id].TryEnter(session_id)) {
				if (debug::DisplaysMSG()) {
					std::print("ID: {} has joined Party: {}.\n", session_id, p.id);
				}

				(*sessions_)[session_id].Push<packet::SCResult>(true);
				(*sessions_)[session_id].SetPartyID(p.id);
				auto partner_id = parties_[p.id].GetPartnerID(session_id);

				if ((*sessions_).TryAccess(partner_id)) {
					auto& pos = (*sessions_)[session_id].GetPlayer().GetPostion();
					(*sessions_)[partner_id].Push<packet::SCNewEntity>(
						entity::kPartnerID, pos.x, pos.y, pos.z, entity::Type::kPlayer);
					(*sessions_).EndAccess(partner_id);
				}
			}
			else {
				(*sessions_)[session_id].Push<packet::SCResult>(false);
			}
			break;
		}
		case packet::Type::kCSPosition: {
			packet::CSPosition p{ buf.GetData() };

			if (debug::DisplaysMSG()) {
				std::print("ID {}: (x, y, z) = ({}, {}, {})\n", session_id, p.x, p.y, p.z);
			}
			
			(*sessions_)[session_id].GetPlayer().SetPosition(p.x, p.y, p.z);
			auto party_id{ (*sessions_)[session_id].GetPartyID() };
			auto partner_id = parties_[party_id].GetPartnerID(session_id);

			if ((*sessions_).TryAccess(partner_id)) {
				(*sessions_)[partner_id].Push<packet::SCPosition>(entity::kPartnerID, p.x, p.y, p.z);
				(*sessions_).EndAccess(partner_id);
			}
			break;
		}
		case packet::Type::kCSLeaveParty: {
			auto party_id = (*sessions_)[session_id].GetPartyID();
			if (party_id < 0 or party_id >= parties_.size()) {
				break;
			}
			parties_[party_id].Exit(session_id);
			break;
		}
		default: {
			std::print("[Error] Unknown Packet: {}\n", static_cast<int>(type));
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