#pragma once
#include <atomic>
#include <array>
#include <vector>
#include <tuple>
#include "concurrent_ds.h"
#include "session.h"

class Party {
public:
	using ID = unsigned short;

	Party() noexcept : num_player_{}, player_ids_{ kEmpty, kEmpty }, id_{} {}

	bool TryEnter(ID id) noexcept;

	ID GetPartnerID(ID id) const noexcept {
		for (auto i : player_ids_) {
			if (i != id) {
				return i;
			}
		}
		return kEmpty;
	}
	void Exit(ID id) noexcept {
		for (auto& i : player_ids_) {
			if (i == id and CAS(&i, id, kEmpty)) {
				if (debug::DisplaysMSG()) {
					std::print("[MSG] ID: {} has left the party.\n", id);
				}
			}
		}
	}
	auto GetPartyMembers() const noexcept {
		auto dword = GetDWord();
		return std::array<ID, 2>{ static_cast<ID>(dword & 0x0000'FFFF), static_cast<ID>((dword & 0xFFFF'0000) >> 16) };
	}
	bool IsAssembled() const noexcept {
		auto members = GetPartyMembers();
		if (members[0] != kEmpty and members[1] != kEmpty) {
			return true;
		}
		return false;
	}
	void SetID(ID id) {
		id_ = id;
	}
private:
	bool CAS(volatile ID* mem, ID expected, ID desired) noexcept {
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<ID>*>(mem), &expected, desired);
	}
	unsigned GetDWord() const noexcept {
		return *reinterpret_cast<const unsigned*>(&player_ids_);
	}
	static constexpr ID kEmpty{ 0xFFFF };
	static constexpr ID kMaxPlayer{ 2 };
	std::array<volatile ID, kMaxPlayer> player_ids_;
	std::atomic_uchar num_player_;
	int id_;
};

inline std::vector<Party> parties(client::GetMaxClients() / 2);