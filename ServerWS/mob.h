//---------------------------------------------------
// 
// mob.h - 몬스터 엔티티 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <print>
#include <unordered_map>
#include "random_number_generator.h"
#include "lua_script.h"
#include "timer.h"
#include "timer_thread.h"
#include "entity.h"
#include "fsm.h"

namespace entity {
	namespace mob {
		inline std::unordered_map<int, Position> spawn_points;
		inline short hp_default;
		inline short hp_diff;

		inline float ai_activation_range;
		inline float acquisition_range;
		inline float attack_range;
		inline float ai_activation_range_sq;
		inline float acquisition_range_sq;
		inline float attack_range_sq;

		inline float attack_angle;
		inline short damage;

		inline float attack_cooldown;
		inline float hitstop_time;

		inline float vel_chase;
		inline float vel_wander;

		inline void LoadData() noexcept
		{
			lua::Script settings{ "scripts/mob_settings.lua" };
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

			mob::attack_angle = settings.GetConstant<float>("ATTACK_ANGLE");
			mob::damage = settings.GetConstant<short>("DAMAGE");

			mob::attack_cooldown = settings.GetConstant<float>("ATTACK_COOLDOWN");
			mob::hitstop_time = settings.GetConstant<float>("HITSTOP_TIME");

			mob::vel_wander = settings.GetConstant<float>("VEL_WANDER");
			mob::vel_chase = settings.GetConstant<float>("VEL_CHASE");
		}
	}

	class Mob : public Base {
	public:
		Mob(ID id, float x, float y, float z, short hp_diff) noexcept
			: Base{ id, x, y, z, hp_diff }, state_{ fsm::State::kAIDisabled }
			, target_pos_{}, target_id_{}, hitstop_time_{}, is_waken_up_{} {
			SetType(Type::kMob);
		}
		void Decide(int p1_id, int p2_id, const Position& p1_pos, const Position& p2_pos) noexcept;
		void Act(float time, const std::unordered_map<int, Position>& positions_in_region) noexcept;
		auto GetState() const noexcept { return state_; }
		float GetMoveTime(float time) noexcept {
			return move_timer_.GetDuration(time);
		}
		bool GetDamaged(short damage) noexcept {
			auto r = Base::GetDamaged(damage);
			hitstop_time_ = 1.0f;
			attack_timer_.ResetTimePoint();
			return r;
		}

		void WakeUp() noexcept {
			if (is_waken_up_) {
				return;
			}
			bool expected{ false };
			if (true == is_waken_up_.compare_exchange_strong(expected, true)) {
				timer::event_pq->Emplace(GetID(), 0, Operation::kUpdateMobAI);
			}
		}

		void Print() noexcept {
			std::print("id {}: (x, y, z) = ({}, {}, {}), region: {}, flag = {:08b}\n",
				GetID(), GetPosition().x, GetPosition().y, GetPosition().z, region_, GetFlag());
		}

		auto GetTargetID() const noexcept {
			return target_id_;
		}

	private:
		void Move(float time, const std::unordered_map<int, Position>& positions_in_region) noexcept;
		void Attack() noexcept;

		int target_id_;
		Position target_pos_;
		fsm::State state_;
		std::atomic_bool is_waken_up_;
		Timer move_timer_;
		Timer attack_timer_;
		float hitstop_time_;
	};

	inline bool CanSee(const Position& pos1, const Position& pos2)
	{
		return GetDistance2DSq(pos1, pos2) <= mob::ai_activation_range_sq;
	}
}