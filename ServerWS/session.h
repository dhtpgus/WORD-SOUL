//---------------------------------------------------
// 
// session.h - client::Session 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>
#include <shared_mutex>
#include "buffer.h"
#include "packet.h"
#include "player.h"
#include "lf_relaxed_queue.h"
#include "debug.h"
#include "over_ex.h"

namespace client {
	class Session {
	public:
		Session() = delete;
		Session(int id, SOCKET sock, HANDLE iocp) noexcept
			: overlapped_{}, sock_{ sock }, buf_recv_{}, wsabuf_recv_{}, player_{},
			rq_{ thread::GetNumWorker() }, wsabuf_send_{} {
			Reset(id, sock, iocp);
			wsabuf_recv_.len = (ULONG)kBufferSize;
			wsabuf_send_[0].buf = reinterpret_cast<char*>(free_list<packet::SCCheckConnection>.Get());
			wsabuf_send_[0].len = sizeof(packet::SCCheckConnection);
		}
		~Session() noexcept {
			packet::Collect(wsabuf_send_[0].buf);
			Delete();
		}
		Session(const Session&) = delete;
		Session(Session&&) noexcept = default;
		Session& operator=(const Session&) = delete;
		Session& operator=(Session&&) noexcept = default;

		void Receive() noexcept {
			static DWORD flags = 0;
			wsabuf_recv_.buf = const_cast<char*>(buf_recv_.GetData());
			WSARecv(sock_, &wsabuf_recv_, 1, nullptr, &flags, &overlapped_, nullptr);
		}

		template<class Packet, class... Value>
		void Push(Value... value) noexcept {
			rq_.Emplace<Packet>(value...);
		}

		int Send() noexcept {
			DWORD num_packet{ 1 };
			while (true) {
				if (num_packet >= 100) {
					break;
				}
				packet::Base* packet = rq_.Pop();
				if (packet == lf::kPopFailed) {
					break;
				}
				wsabuf_send_[num_packet].buf = reinterpret_cast<char*>(packet);
				wsabuf_send_[num_packet].len = packet->size + sizeof(packet::Base);
				num_packet += 1;
			}

			auto ox = free_list<OverEx>.Get(Operation::kSend);
			int ret = WSASend(sock_, &wsabuf_send_[0], num_packet, nullptr, 0, &ox->over, 0);

			for (DWORD i = 1; i < num_packet; ++i) {
				packet::Collect(wsabuf_send_[i].buf);
			}

			return ret == 0;
		}

		void Reset(int id, SOCKET sock, HANDLE iocp) noexcept {
			if (debug::DisplaysMSG()) {
				std::print("[Info] ID: {} has joined the game.\n", GetID());
			}
			sock_ = sock;
			buf_recv_.ResetCursor();
			id_ = id;
			party_id_ = -1;
			CreateIoCompletionPort((HANDLE)sock_, iocp, id_, 0);
			wsabuf_recv_.buf = const_cast<char*>(buf_recv_.GetData());
		}

		void Delete() noexcept {
			if (debug::DisplaysMSG()) {
				std::print("[Info] ID: {} has left the game.\n", GetID());
			}
			packet::Base* packet{};
			while (true) {
				packet = rq_.Pop();
				if (nullptr == packet) {
					break;
				}
				packet::Collect(packet);
			}
		}

		BufferRecv& GetBuffer() noexcept { return buf_recv_; }
		int GetID() const noexcept { return id_; }
		SOCKET GetSocket() const noexcept { return sock_; }
		int GetPartyID() const noexcept { return party_id_; }
		void SetPartyID(int id) noexcept { party_id_ = id; }
		auto& GetPlayer() noexcept { return player_; }
	private:
		OVERLAPPED overlapped_;
		SOCKET sock_;
		BufferRecv buf_recv_;
		WSABUF wsabuf_recv_;
		std::array<WSABUF, 100> wsabuf_send_;
		int id_;
		int party_id_;
		entity::Player player_;
		lf::RelaxedQueue<packet::Base> rq_;
	};
}