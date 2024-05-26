#include "party.h"
#include "entity_manager.h"

bool Party::TryEnter(ID id) noexcept
{
	if (num_player_ >= kMaxPlayer) {
		return false;
	}
	for (int i = 0; i < kMaxPlayer; ++i) {
		if (CAS(&player_ids_[i], kEmpty, id)) {
			num_player_ += 1;

			if (num_player_ == kMaxPlayer) {
				entity::managers[id_].SpawnMonsters();
			}

			return true;
		}
	}
	return false;
}