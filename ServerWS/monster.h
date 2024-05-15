//---------------------------------------------------
// 
// monster.h - 몬스터 엔티티 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <unordered_map>
#include "random_number_generator.h"
#include "lua_script.h"
#include "entity.h"
#include "fsm.h"

namespace entity {
	class Monster : public Base {
	public:
		Monster(ID id, float x, float y, float z, short hp) noexcept
			: Base{ id, x, y, z, hp }, state_{ fsm::State::kAIDisabled }, target_pos_{} {
			SetType(Type::kMonster);
		}
		void Decide(const Position& p1_pos, const Position& p2_pos) noexcept;
		void Act(float time) noexcept;
		auto GetState() const noexcept { return state_; }

		constexpr static auto kWanderSpeed{ 300.0f };
		constexpr static auto kChaseSpeed{ 150.0f };
	private:
		void Move(float time) noexcept;
		void Attack() noexcept;
		static char GetFlag(Base& en) noexcept {
			char flag{};
			auto m = reinterpret_cast<Monster*>(&en);
			flag |= static_cast<char>(m->GetState());

			return flag;
		}

		constexpr static auto kAcquisitionRange{ 1000.0f };
		constexpr static auto kAcquisitionRangeSq{ kAcquisitionRange * kAcquisitionRange };
		constexpr static auto kAttackRange{ 30.0f };
		constexpr static auto kAttackRangeSq{ kAttackRange * kAttackRange };
		constexpr static auto kAIActivationRange{ 3000.0f };
		constexpr static auto kAIActivationRangeSq{ kAIActivationRange * kAIActivationRange };

		fsm::State state_;
		Position target_pos_;
	};

	inline std::unordered_map<int, Position> monster_spawn_points;
	inline short monster_hp;
	inline short monster_hp_diff;

	inline void LoadMonsterData() noexcept
	{
		lua::Script monster_settings{ "scripts/monster_settings.lua" };
		const int kNumPoints{ monster_settings.GetConstant<int>("NUM_POINTS") };

		for (int i = 0; i < kNumPoints; ++i) {
			auto r_ptr = monster_settings.CallFunction<int, float>("get_spawn_point", 2, { i + 1 });
			auto& r = *r_ptr;
			r[0] += rng.Rand(-5.0f, 5.0f);
			r[1] += rng.Rand(-5.0f, 5.0f);
			monster_spawn_points.emplace(i, Position{ r[0], r[1], 0.0f });
		}

		monster_hp = monster_settings.GetConstant<short>("HP");
		monster_hp_diff = monster_settings.GetConstant<short>("HP_DIFF");
	}
}