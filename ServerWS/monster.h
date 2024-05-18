//---------------------------------------------------
// 
// monster.h - 몬스터 엔티티 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <unordered_map>
#include "random_number_generator.h"
#include "lua_script.h"
#include "timer.h"
#include "entity.h"
#include "fsm.h"

namespace entity {
	class Monster : public Base {
	public:
		Monster(ID id, float x, float y, float z, short hp) noexcept
			: Base{ id, x, y, z, hp }, state_{ fsm::State::kAIDisabled }
			, target_pos_{}, stun_time_{} {
			SetType(Type::kMonster);
		}
		void Decide(const Position& p1_pos, const Position& p2_pos) noexcept;
		void Act(float time) noexcept;
		auto GetState() const noexcept { return state_; }
		float GetMoveTime(float time) noexcept {
			return move_timer_.GetDuration(time);
		}
		void GetDamaged() noexcept {
			stun_time_ = 1.0f;
			attack_timer_.ResetTimePoint();
		}

	private:
		void Move(float time) noexcept;
		void Attack() noexcept;

		Position target_pos_;
		fsm::State state_;
		Timer move_timer_;
		Timer attack_timer_;
		float hitstop_time_;
	};

	namespace monster {
		inline std::unordered_map<int, Position> spawn_points;
		inline short hp_default;
		inline short hp_diff;

		inline float ai_activation_range;
		inline float acquisition_range;
		inline float attack_range;
		inline float ai_activation_range_sq;
		inline float acquisition_range_sq;
		inline float attack_range_sq;

		inline float attack_cooldown;
		inline float hitstop_time;

		inline float vel_chase;
		inline float vel_wander;

		inline void LoadData() noexcept
		{
			lua::Script settings{ "scripts/monster_settings.lua" };
			const int kNumPoints{ settings.GetConstant<int>("NUM_POINTS") };

			for (int i = 0; i < kNumPoints; ++i) {
				auto r_ptr = settings.CallFunction<int, float>("get_spawn_point", 2, { i + 1 });
				auto& r = *r_ptr;
				r[0] += rng.Rand(-5.0f, 5.0f);
				r[1] += rng.Rand(-5.0f, 5.0f);
				spawn_points.emplace(i, Position{ r[0], r[1], 0.0f });
			}

			hp_default = settings.GetConstant<short>("HP_DEFAULT");
			hp_diff = settings.GetConstant<short>("HP_DIFF");

			ai_activation_range = settings.GetConstant<float>("AI_ACTIVATION_RANGE");
			acquisition_range = settings.GetConstant<float>("ACQUISITION_RANGE");
			attack_range = settings.GetConstant<float>("ATTACK_RANGE");

			ai_activation_range_sq = ai_activation_range * ai_activation_range;
			acquisition_range_sq = acquisition_range * acquisition_range;
			attack_range_sq = attack_range * attack_range;

			monster::attack_cooldown = settings.GetConstant<float>("ATTACK_COOLDOWN");
			monster::hitstop_time = settings.GetConstant<float>("HITSTOP_TIME");

			monster::vel_wander = settings.GetConstant<float>("VEL_WANDER");
			monster::vel_chase = settings.GetConstant<float>("VEL_CHASE");
		}
	}
}