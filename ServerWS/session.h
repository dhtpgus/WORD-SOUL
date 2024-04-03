//---------------------------------------------------
// 
// session.h - client::Session 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>
#include <shared_mutex>
#include "packet.h"
#include "player.h"
#include "lf_relaxed_queue.h"
#include "free_list.h"
#include "debug.h"
#include "over_ex.h"

namespace client {
	class Session {
	public:
		Session() = delete;
		Session(int id, SOCKET sock, HANDLE iocp) noexcept
			: overlapped_{}, sock_{ sock }, buf_recv_{}, recv_bytes_{}, send_bytes_{},
			wsabuf_recv_{}, id_{ id }, player_{ new entity::Player }, party_id_{ -1 },
			rq_{ thread::GetNumWorker() }, wsabuf_send_{} {
			wsabuf_recv_.buf = buf_recv_;
			wsabuf_recv_.len = (ULONG)kBufferSize;
			CreateIoCompletionPort((HANDLE)sock_, iocp, id, 0);
			if (debug::IsDebugMode()) {
				std::print("[Info] ID: {} has joined the game.\n", GetID());
			}
			wsabuf_send_[0].buf = reinterpret_cast<char*>(new packet::SCCheckConnection{});
			wsabuf_send_[0].len = sizeof(packet::SCCheckConnection);
		}
		~Session() noexcept {
			delete player_;
			delete wsabuf_send_[0].buf;
			if (debug::IsDebugMode()) {
				std::print("[Info] ID: {} has left the game.\n", GetID());
			}
			closesocket(sock_);
		}
		Session(const Session&) = delete;
		Session(Session&&) noexcept = default;
		Session& operator=(const Session&) = delete;
		Session& operator=(Session&&) noexcept = default;

		void Receive() noexcept {
			static DWORD flags = 0;
			wsabuf_recv_.buf = buf_recv_;
			WSARecv(sock_, &wsabuf_recv_, 1, &recv_bytes_, &flags, &overlapped_, nullptr);
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
			/*if (0 == num_packet) {
				return true;
			}*/

			auto ox = free_list::ox.Get<OverEx>(Operation::kSend);
			int ret = WSASend(sock_, wsabuf_send_, num_packet, nullptr, 0, &ox->over, 0);

			for (DWORD i = 1; i < num_packet; ++i) {
				packet::Free(wsabuf_send_[i].buf);
			}

			return ret == 0;
		}

		const char* GetBuffer() const noexcept { return buf_recv_; }
		int GetID() const noexcept { return id_; }
		SOCKET GetSocket() const noexcept { return sock_; }
		int GetPartyID() const noexcept { return party_id_; }
		void SetPartyID(int id) noexcept { party_id_ = id; }
		auto& GetPlayer() noexcept { return *player_; }
	private:
		static constexpr size_t kBufferSize = 1024;
		OVERLAPPED overlapped_;
		SOCKET sock_;
		char buf_recv_[kBufferSize];
		DWORD recv_bytes_;
		DWORD send_bytes_;
		WSABUF wsabuf_recv_;
		WSABUF wsabuf_send_[100];
		int id_;
		int party_id_;
		entity::Player* player_;
		lf::RelaxedQueue<packet::Base> rq_;
	};
}