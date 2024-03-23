#pragma once
#include <atomic>
#include "lf_array.h"
#include "entity_manager.h"

class Party {
public:
	Party() : num_player_{}, player_id_{ -1, -1 }, entities_{} {}
	void InitEntityManager(int entity_num, int thread_num) {
		entities_ = std::make_shared<entity::Manager>(entity_num, thread_num);
	}
	bool Enter(int id) {
		if (num_player_ >= kMaxPlayer) {
			return false;
		}
		for (int i = 0; i < kMaxPlayer; ++i) {
			if (CAS(&player_id_[i], -1, id)) {
				num_player_ += 1;
				return true;
			}
		}
		return false;
	}
private:
	bool CAS(volatile int* mem, int expected, int desired) {
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic_int*>(mem), &expected, desired);
	}
	static constexpr auto kMaxPlayer{ 2 };
	volatile int player_id_[kMaxPlayer];
	std::atomic<char> num_player_;
	std::shared_ptr<entity::Manager> entities_;
};