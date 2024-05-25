//---------------------------------------------------
// 
// entity_manager.h - 엔티티 관리자 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "lf_array.h"
#include "thread.h"
#include "entity.h"
#include "player.h"
#include "mob.h"
#include "boss.h"
#include "world_map.h"
#include "lf_base15_tree.h"

namespace entity {
	inline constexpr auto kMaxEntities{ 100 };

	class Manager : public lf::Array<Base> {
	public:
		Manager() noexcept
			: lf::Array<Base>{ kMaxEntities, thread::GetNumWorkers() }
			, entities_in_region_{} {}

		void UpdateEntityPosition(int id, float x, float y, float z) noexcept {
			if (TryAccess(id)) {
				Get(id).SetPosition(x, y, z);
				EndAccess(id);
			}
		}
		void SpawnMonsters() noexcept {
			for (auto& [i, pos] : mob::spawn_points) {
				short rand_hp = mob::hp_default + rng.Rand<short>(-mob::hp_diff, mob::hp_diff);
				Allocate<Mob>(pos.x, pos.y, pos.z, rand_hp);
				//std::print("{}, ({}, {})\n", world_map.FindRegion(pos), pos.x, pos.y);
			}
		}
		void Update(int id1, int id2, const Position& pos1, const Position& pos2) noexcept {
			std::array<std::unordered_map<int, Position>, WorldMap::kNumRegions> positions_in_region{};
			std::array<std::vector<int>, WorldMap::kNumRegions> id_in_region{};
			for (int i = 0; i < WorldMap::kNumRegions; ++i) {
				entities_in_region_[i].GetElements(id_in_region[i]);
				for (auto id : id_in_region[i]) {
					if (TryAccess(id)) {
						positions_in_region[i].try_emplace(id, Get(id).GetPostion());
						EndAccess(id);
					}
				}
			}

			for (int i = 0; i < kMaxEntities; ++i) {
				if (TryAccess(i)) {
					auto& e = Get(i);
					if (Type::kMob == e.GetType()) {
						auto m = reinterpret_cast<Mob*>(&e);
						auto time = m->GetMoveTime(1.0f / 80);
						if (time == 0.0f) {
							continue;
						}
						m->Decide(id1, id2, pos1, pos2);
						m->Act(time, positions_in_region[m->region_]);
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