#pragma once
#include <atomic>
#include <array>
#include <tuple>
#include "lf_array.h"
#include "entity_manager.h"

class Party {
public:
	using ID = unsigned short;
	Party() noexcept : num_player_{}, player_id_{ kEmpty, kEmpty }, entities_{} {}
	void InitEntityManager(int entity_num, int thread_num) {
		entities_ = std::make_shared<entity::Manager>(entity_num, thread_num);
	}
	bool TryEnter(int id) noexcept {
		if (num_player_ >= kMaxPlayer) {
			return false;
		}
		for (int i = 0; i < kMaxPlayer; ++i) {
			if (CAS(&player_id_[i], kEmpty, id)) {
				num_player_ += 1;
				return true;
			}
		}
		return false;
	}
	int GetPartnerID(int id) const noexcept {
		for (auto i : player_id_) {
			if (i != id) {
				return id;
			}
		}
		return kEmpty;
	}
	void Exit(int id) noexcept {
		for (auto& i : player_id_) {
			if (i == id and CAS(&i, id, kEmpty)) {
				if (debug::IsDebugMode()) {
					std::print("[MSG] ID: {} has left the party.\n", id);
				}
			}
		}
	}
	auto GetPartyMembers() const noexcept {
		return player_id_;
	}
private:
	bool CAS(volatile ID* mem, ID expected, ID desired) noexcept {
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<ID>*>(mem), &expected, desired);
	}
	static constexpr ID kEmpty{ 0xFFFF };
	static constexpr ID kMaxPlayer{ 2 };
	std::array<volatile ID, kMaxPlayer> player_id_;
	std::atomic_uchar num_player_;
	std::shared_ptr<entity::Manager> entities_;
};