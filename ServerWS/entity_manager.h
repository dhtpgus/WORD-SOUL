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
#include "lf_base15_tree.h"

namespace entity {
	inline constexpr auto kMaxEntities{ 100 };

	class Manager : public lf::Array<Base> {
	public:
		Manager() noexcept
			: lf::Array<Base>{ kMaxEntities, thread::kNumWorkers } {}
		void UpdateEntityPosition(int id, float x, float y, float z) noexcept {
			if (TryAccess(id)) {
				Get(id).SetPosition(x, y, z);
				EndAccess(id);
			}
		}
		void Spawn() noexcept {
			Allocate<Monster>(500.0f, 500.0f, 0.0f, (short)200);
		}
		void Update(const Position& p1, const Position& p2, float time) noexcept {
			for (int i = 0; i < kMaxEntities; ++i) {
				if (TryAccess(i)) {
					std::print("{} {}\n", i, (int)Get(i).GetType());
					if (Type::kMonster == Get(i).GetType()) {
						auto m = reinterpret_cast<Monster*>(&Get(i));
						m->Decide(p1, p2);
						m->Act(time);
					}
					EndAccess(i);
				}
			}
		}
	};

	inline std::vector<Manager> managers(client::GetMaxClients() / 2);
}