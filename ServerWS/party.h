#pragma once
#include <atomic>
#include "lf_array.h"
#include "entity_manager.h"

class Party {
public:
	Party() : num_player_{}, player_id_{ kEmpty, kEmpty }, entities_{} {}
	void InitEntityManager(int entity_num, int thread_num) {
		entities_ = std::make_shared<entity::Manager>(entity_num, thread_num);
	}
	bool TryEnter(int id) {
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
	int GetPartnerID(int id) const {
		for (auto i : player_id_) {
			if (i != id) {
				return id;
			}
		}
		return kEmpty;
	}
	void Exit(int id) {
		for (auto& i : player_id_) {
			if (i == id and CAS(&i, id, kEmpty)) {
				if (debug::IsDebugMode()) {
					std::print("[MSG] ID: {} has left the party.\n", id);
				}
			}
		}
	}
private:
	bool CAS(volatile int* mem, int expected, int desired) {
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic_int*>(mem), &expected, desired);
	}
	static constexpr auto kEmpty{ -1 };
	static constexpr auto kMaxPlayer{ 2 };
	volatile int player_id_[kMaxPlayer];
	std::atomic_uchar num_player_;
	std::shared_ptr<entity::Manager> entities_;
};