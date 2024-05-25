//---------------------------------------------------
// 
// session.h - client::Session 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <print>
#include <thread>
#include "buffer.h"
#include "packet.h"
#include "player.h"
#include "lf_relaxed_queue.h"
#include "debug.h"
#include "over_ex.h"
#include "lua_script.h"

namespace client {
	class Session {
	public:
		Session() = delete;
		Session(int id, SOCKET sock, HANDLE iocp) noexcept
			: ox_{ Operation::kRecv }, sock_{ sock }, wsabuf_recv_{}, player_{},
			rq_{ thread::GetNumWorkers() } {
			Reset(id, sock, iocp);
			wsabuf_recv_.len = (ULONG)kBufferSize;
			wsabuf_recv_.buf = buf_recv_.GetRecvPoint();
		}
		~Session() noexcept {
			Delete();
		}
		Session(const Session&) = delete;
		Session(Session&&) noexcept = default;
		Session& operator=(const Session&) = delete;
		Session& operator=(Session&&) noexcept = default;

		void Receive() noexcept {
			static DWORD flags = 0;
			WSARecv(sock_, &wsabuf_recv_, 1, nullptr, &flags, &ox_.over, nullptr);
		}

		template<class Packet, class... Value>
		bool Emplace(Value... value) noexcept {
			//rq_.Emplace<Packet>(value...);
			auto p = free_list<Packet>.Get(value...);

			WSABUF wb;
			wb.buf = reinterpret_cast<char*>(p);
			wb.len = (p->size + 2);
			auto ox = free_list<OverEx>.Get(Operation::kSend);

			auto r = WSASend(sock_, &wb, 1, nullptr, 0, &ox->over, 0);
			free_list<Packet>.Collect(p);
			return r == 0;
		}

		void Reset(int id, SOCKET sock, HANDLE iocp) noexcept {
			sock_ = sock;
			buf_recv_.ResetCursor();
			id_ = id;
			party_id_ = -1;
			CreateIoCompletionPort((HANDLE)sock_, iocp, id_, 0);
			wsabuf_recv_.buf = const_cast<char*>(buf_recv_.GetData());
			if (debug::DisplaysMSG()) {
				std::print("[Info] (ID: {}) has joined the game.\n", GetID());
			}
		}

		void Delete() noexcept {
			if (debug::DisplaysMSG()) {
				std::print("[Info] (ID: {}) has left the game.\n", GetID());
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
		OverEx ox_;
		SOCKET sock_;
		BufferRecv buf_recv_;
		WSABUF wsabuf_recv_;
		int id_;
		int party_id_;
		entity::Player player_;
		lf::RelaxedQueue<packet::Base, 1e-4> rq_;
	};

	inline int GetMaxClients() noexcept {
		static int max_clients;
		static bool has_read;
		if (has_read) {
			return max_clients;
		}

		lua::Script server_settings{ "scripts/server_settings.lua" };
		max_clients = server_settings.GetConstant<int>("MAX_CLIENTS");
		has_read = true;

		std::print("[Info] Loading...\n");
		if (debug::DisplaysMSG()) {
			std::print("[Info] Max Clients: {}\n", max_clients);
		}

		return max_clients;
	}
}