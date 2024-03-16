//---------------------------------------------------
// 
// client_socket.h - client::Socket 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>
#include <shared_mutex>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "packet.h"
#include "lf_relaxed_queue.h"
#include "debug.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

namespace client {
	class Socket {
	public:
		Socket() = delete;
		Socket(int id, SOCKET sock, HANDLE iocp)
			: overlapped_{}, sock_{ sock }, buf_recv_{}, recv_bytes_{}, send_bytes_{},
			wsabuf_recv_{}, id_{ id }, player_id_{}, rq_{ thread::GetNumWorker() }, wsabuf_send_{} {
			wsabuf_recv_.buf = buf_recv_;
			wsabuf_recv_.len = (ULONG)kBufferSize;
			CreateIoCompletionPort((HANDLE)sock_, iocp, id, 0);
			if (debug::IsDebugMode()) {
				std::print("[Info] ID: {} has joined.\n", GetID());
			}
		}
		~Socket() {
			if (debug::IsDebugMode()) {
				std::print("[Info] ID: {} has left.\n", GetID());
			}
			closesocket(sock_);
		}
		Socket(const Socket&) = delete;
		Socket(Socket&&) = default;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = default;

		void Receive() {
			static DWORD flags = 0;
			wsabuf_recv_.buf = buf_recv_;
			WSARecv(sock_, &wsabuf_recv_, 1, &recv_bytes_, &flags, &overlapped_, nullptr);
		}

		template<class Packet, class... Value>
		void Push(Value... value) {
			rq_.Emplace<Packet>(value...);
		}

		int Send() {
			DWORD num_packet{};
			while (true) {
				packet::Base* packet = rq_.Pop();
				if (packet == lf::kPopFailed or num_packet >= 100) {
					break;
				}
				wsabuf_send_[num_packet].buf = reinterpret_cast<char*>(packet);
				wsabuf_send_[num_packet].len = packet->size + sizeof(packet::Base);
				num_packet += 1;
			}
			if (0 == num_packet) {
				return 0;
			}
			int ret = WSASend(sock_, wsabuf_send_, num_packet, &send_bytes_, 0, 0, 0);

			for (DWORD i = 0; i < num_packet; ++i) {
				packet::Free(wsabuf_send_[i].buf);
			}

			return ret;
		}

		const char* GetBuffer() const {
			return buf_recv_;
		}

		int GetID() const {
			//return (id_ & 0x7FFF'FFFF);
			return id_;
		}

		SOCKET GetSocket() const {
			return sock_;
		}

		int GetPlayerID(int id) const {
			return player_id_;
		}

		void SetPlayerID(int id) {
			player_id_ = id;
		}

		/*bool IsLogicallyDeleted() const {
			return (id_ & 0x8000'0000) != 0;
		}
		void DeleteLogically() {
			id_ |= 0x8000'0000;
		}
		static constexpr bool is_dangerous_to_delete{ true };*/

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
		int player_id_;
		lf::RelaxedQueue<packet::Base> rq_;
	};
}