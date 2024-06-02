#include <print>
#include "mob.h"
#include "random_number_generator.h"
#include "world_map.h"
#include "a_star.h"

namespace entity {
	void Mob::Decide(int p1_id, int p2_id, const Position& p1_pos, const Position& p2_pos) noexcept
	{
		auto distance_sq_p1 = GetDistance2DSq(p1_pos, GetPosition());
		auto distance_sq_p2 = GetDistance2DSq(p2_pos, GetPosition());
		int target = distance_sq_p1 > distance_sq_p2 ? 2 : 1;
		target_id_ = target == 1 ? p1_id : p2_id;

		auto distance_sq = target == 1 ? distance_sq_p1 : distance_sq_p2;
		const float kPi = acosf(-1);

		if (distance_sq < mob::attack_range_sq) {
			target_pos_ = target == 1 ? p1_pos : p2_pos;
			if (state_ != fsm::State::kAttack) {
				attack_timer_.ResetTimePoint();
			}
			state_ = fsm::State::kAttack;
		}
		else if (distance_sq < mob::acquisition_range_sq) {
			target_pos_ = target == 1 ? p1_pos : p2_pos;
			state_ = fsm::State::kChase;
		}
		else if (distance_sq < mob::ai_activation_range_sq){
			float rad = fmod(dir_ - rng.Rand(-0.3f, 0.3f), 2 * kPi);
			auto [x, y] = target_pos_.GetXY();
			target_pos_.SetXY(x + 200.0f * cosf(rad), y + 200.0f * sinf(rad));
			state_ = fsm::State::kWander;
		}
		else {
			state_ = fsm::State::kAIDisabled;
			is_waken_up_ = false;
		}

		//std::print("_{}, state: {}\n", GetID(), (int)state_);
	}

	void Mob::Act(float time, const std::unordered_map<int, Position>& positions_in_region) noexcept
	{
		switch (state_)
		{
		case fsm::State::kWander:
			Move(time, positions_in_region);
			break;
		case fsm::State::kChase:
			Move(time, positions_in_region);
			break;
		case fsm::State::kAttack:
			Attack();
			break;
		default:
			break;
		}
	}

	void Mob::Move(float time, const std::unordered_map<int, Position>& positions_in_region) noexcept
	{
		const float kVel{ (fsm::State::kChase == state_) ? mob::vel_chase : mob::vel_wander };

		auto new_pos = a_star::GetNextPosition(GetPosition(), dir_, target_pos_, time, kVel, positions_in_region);
		SetPosition(new_pos.x, new_pos.y, new_pos.z);
	}

	void Mob::Attack() noexcept
	{
		auto& pos = GetPosition();

		dir_ = atan2f(target_pos_.y - pos.y, target_pos_.x - pos.x);

		auto local_flag = flag_;
		local_flag &= ~0b11100;
		if (attack_timer_.GetDuration(mob::attack_cooldown) != 0.0f) {
			local_flag |= 0b00100;
		}
		flag_ = local_flag;
	}
}