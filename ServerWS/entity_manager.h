//---------------------------------------------------
// 
// entity_manager.h - 엔티티 관리자 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "thread.h"
#include "entity.h"
#include "session.h"
#include "mob.h"
#include "party.h"
#include "boss.h"
#include "world_map.h"
#include "concurrent_ds.h"

namespace entity {
	inline constexpr auto kMaxEntities{ 100 };

	class Manager : public concurrent::Array<entity::Base> {
	public:
		Manager() noexcept
			: concurrent::Array<Base>{ kMaxEntities, thread::GetNumWorkers() }
			, entities_in_region_{}, id_{ -1 } {}

		void SetID(int id) noexcept {
			id_ = id;
		}

		void UpdateEntityPosition(int id, float x, float y, float z) noexcept {
			if (TryAccess(id)) {
				Get(id).SetPosition(x, y, z);
				EndAccess(id);
			}
		}
		void SpawnMonsters() noexcept {
			for (auto& [i, pos] : mob::spawn_points) {
				short rand_hp = mob::hp_default + rng.Rand<short>(-mob::hp_diff, mob::hp_diff);
				int m_id = Allocate<Mob>(pos.x, pos.y, pos.z, rand_hp);

				if (TryAccess(m_id)){
					auto& m = *reinterpret_cast<Mob*>(&Get(m_id));
					int r = world_map.FindRegion(m.GetPosition());
					m.region_ = r;
					MarkRegion(r, m.GetID());
					EndAccess(m_id);
				}
				ai_locks[m_id].Unlock();

				//std::print("{}, ({}, {})\n", world_map.FindRegion(pos), pos.x, pos.y);
			}
		}
		bool UpdateAI(entity::ID id) noexcept {
			lf::CASLockGuard lg{ ai_locks[id] };
			if (lg) {
				return false;
			}

			auto players{ parties[id_].GetPartyMembers() };
			Position players_pos[players.size()]{};

			for (int i = 0; i < players.size(); ++i) {
				if (sessions.TryAccess(players[i])) {
					players_pos[i] = sessions[players[i]].GetPlayer().GetPosition();
				}
				else {
					return false;
				}
			}

			if (TryAccess(id)) {
				auto& m = *reinterpret_cast<Mob*>(&Get(id));
				std::unordered_map<int, Position> positions_in_region{};
				std::vector<int> id_in_region{};
				id_in_region.reserve(30);

				/*if ((m.GetFlag() & 0b111) == 0b111)
				m.Print();*/

				m.Decide(players[0], players[1], players_pos[0], players_pos[1]);
				if (fsm::State::kAIDisabled == m.GetState()) {
					EndAccess(id);
					return false;
				}

				auto region = m.region_;
				
				if (IsValidRegion(region)) {
					entities_in_region_[region].GetElements(id_in_region);
				}

				/*else {
					m.Print();
				}*/

				for (auto id : id_in_region) {
					if (TryAccess(id)) {
						positions_in_region.try_emplace(id, Get(id).GetPosition());
						EndAccess(id);
					}
				}
				positions_in_region.erase(id);

				m.Act(0.020f, positions_in_region);
				int r = world_map.FindRegion(m.GetPosition());

				if (r != m.region_) {
					MarkRegion(region, r, m.GetID());
				}
				m.region_ = r;

				const auto& m_pos = m.GetPosition();

				for (int i = (int)players.size() - 1; i >= 0; --i) {
					sessions[players[i]].Emplace<packet::SCPosition>(&m);
					sessions.EndAccess(players[i]);
				}

				EndAccess(id);

				return true;
			}

			return false;
		}

		void KillMonster(ID id) {
			ai_locks[id].TryLock();
			ReserveDelete(id);
		}

		template<class Container>
		void GetEntitiesInRegion(int region, Container& con) {
			entities_in_region_[region].GetElements(con);
		}
	private:
		void MarkRegion(int curr_region, ID id) {
			auto& curr_adjs = world_map.GetRegion(curr_region).GetAdjRegions();
			for (auto adj : curr_adjs) {
				entities_in_region_[adj->GetID()].Insert(id);
			}
		}
		void MarkRegion(int prev_region, int curr_region, ID id) {
			if (curr_region < 0 or curr_region >= WorldMap::kNumRegions) {
				if (prev_region < 0 or prev_region >= WorldMap::kNumRegions) {
					return;
				}
				auto& prev_adjs = world_map.GetRegion(prev_region).GetAdjRegions();
				for (auto adj : prev_adjs) {
					entities_in_region_[adj->GetID()].Remove(id);
				}
				return;
			}
			if (prev_region < 0 or prev_region >= WorldMap::kNumRegions) {
				MarkRegion(curr_region, id);
				return;
			}

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

		std::array<concurrent::Set, WorldMap::kNumRegions> entities_in_region_;
		std::array<lf::CASLock, kMaxEntities> ai_locks;
		int id_;
	};

	inline std::vector<Manager> managers(client::GetMaxClients() / 2);

	inline void InitManager() {
		for (int i = 0; i < managers.size(); ++i) {
			managers[i].SetID(i);
		}
	}
}