//---------------------------------------------------
// 
// entity_manager.h - 엔티티 관리자 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <memory>
#include <vector>
#include <memory>
#include "lf_array.h"
#include "thread.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "boss.h"
#include "world_map.h"
#include "lf_base15_tree.h"

namespace entity {
	inline constexpr auto kMaxEntities{ 100 };

	class Manager : public lf::Array<Base> {
	public:
		Manager() noexcept
			: lf::Array<Base>{ kMaxEntities, thread::kNumWorkers }
			, entities_in_region_{} {}

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
					auto& e = Get(i);
					if (Type::kMonster == e.GetType()) {
						auto m = reinterpret_cast<Monster*>(&e);
						m->Decide(p1, p2);
						m->Act(time);
						int r = world_map.FindRegion(m->GetPostion());

						if (r != m->region_) {
							MarkRegion(m->region_, r, m->GetID());
						}
						m->region_ = r;
					}
					EndAccess(i);
				}
			}
		}
		template<class Container>
		void GetActivated(int sector, Container& con) {
			entities_in_region_[sector].GetElements(con);
		}
	private:
		void MarkRegion(int prev_region, int curr_region, ID id) {
			auto& curr_adjs = world_map.GetRegion(curr_region).GetAdjRegions();
			auto& prev_adjs = world_map.GetRegion(prev_region).GetAdjRegions();

			for (auto adj : curr_adjs) {
				if (0 != prev_adjs.count(adj)) {
					entities_in_region_[adj->GetID()].Insert(id);
				}
			}
			for (auto adj : prev_adjs) {
				if (0 != curr_adjs.count(adj)) {
					entities_in_region_[adj->GetID()].Remove(id);
				}
			}
		}

		std::array<lf::Base15Tree, WorldMap::kNumRegions> entities_in_region_;
	};

	inline std::vector<Manager> managers(client::GetMaxClients() / 2);
}