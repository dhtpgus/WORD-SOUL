#include <print>
#include "monster.h"
#include "random_number_generator.h"
#include "world_map.h"
#include "a_star.h"

namespace entity {
	void Monster::Decide(const Position& p1_pos, const Position& p2_pos) noexcept
	{
		auto distance_sq_p1 = GetDistanceSq(p1_pos, GetPostion());
		auto distance_sq_p2 = GetDistanceSq(p2_pos, GetPostion());
		int target = distance_sq_p1 > distance_sq_p2 ? 2 : 1;
		float distance_sq = target == 1 ? distance_sq_p1 : distance_sq_p2;
		const float kPi = acosf(-1);

		if (distance_sq < kAttackRangeSq) {
			target_pos_ = target == 1 ? p1_pos : p2_pos;
			state_ = fsm::State::kAttack;
		}
		else if (distance_sq < kAcquisitionRangeSq) {
			target_pos_ = target == 1 ? p1_pos : p2_pos;
			state_ = fsm::State::kChase;
		}
		else if (distance_sq < kAIActivationRangeSq){
			float rad = rng.Rand(0.0f, 2 * kPi);
			target_pos_.x = GetPostion().x + 200.0f * cosf(rad);
			target_pos_.y = GetPostion().y + 200.0f * sinf(rad);
			state_ = fsm::State::kWander;
		}
		else {
			state_ = fsm::State::kAIDisabled;
		}
	}

	void Monster::Act(float time) noexcept
	{
		switch (state_)
		{
		case fsm::State::kAIDisabled:
			break;
		case fsm::State::kWander:
			Move(time);
			break;
		case fsm::State::kChase:
			Move(time);
			break;
		case fsm::State::kAttack:
			break;
		default:
			break;
		}
	}

	void Monster::Move(float time) noexcept
	{
		const float kSpeed{ (fsm::State::kChase == state_) ? kChaseSpeed, kWanderSpeed };

		auto new_pos = a_star::GetNextPosition(GetPostion(), target_pos_, time, kSpeed);
		SetPosition(new_pos.x, new_pos.y, new_pos.z);
	}

	void Monster::Attack() noexcept
	{
		
	}
}