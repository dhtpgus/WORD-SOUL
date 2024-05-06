//---------------------------------------------------
// 
// entity_manager.h - 엔티티 관리자 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <memory>
#include <vector>
#include "lf_array.h"
#include "thread.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "boss.h"

namespace entity {
	class Manager {
	public:
		Manager() noexcept
			: entities_{ kMaxEntities, thread::kNumWorkers } {
		}
		void UpdateEntityPosition(int id, float x, float y, float z) noexcept {
			if (entities_.TryAccess(id)) {
				entities_[id].SetPosition(x, y, z);
				entities_.EndAccess(id);
			}
		}
		void Spawn() noexcept {
			entities_.Allocate<Monster>(1000.0f, 500.0f, 0.0f, (short)200);
		}
		void Update() noexcept {
			for (int i = 0; i < kMaxEntities; ++i) {
				if (entities_.TryAccess(i)) {
					if (Type::kMonster == entities_[i].GetType()) {
						auto m = reinterpret_cast<Monster*>(&entities_[i]);
						/*m->Decide();
						m->Act();*/
					}
					entities_.EndAccess(i);
				}

			}
		}

	private:
		static constexpr auto kMaxEntities{ 100 };

		using EntityArray = lf::Array<Base>;
		EntityArray entities_;
	};

	inline std::vector<Manager> managers;
}