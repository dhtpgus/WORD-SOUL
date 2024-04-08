#include "monster.h"
#include "random_number_generator.h"
#include "region.h"

namespace entity {
	void Monster::Decide(const Position& p1_pos, const Position& p2_pos) noexcept
	{
		auto distance_sq_p1 = GetDistanceSq(p1_pos, GetPostion());
		auto distance_sq_p2 = GetDistanceSq(p2_pos, GetPostion());
		int target = distance_sq_p1 > distance_sq_p2 ? 2 : 1;
		float distance_sq = target == 1 ? distance_sq_p1 : distance_sq_p2;

		if (distance_sq < kAttackRangeSq) {
			target_pos_ = target == 1 ? p1_pos : p2_pos;
			state_ = ai::State::kAttack;
		}
		else if (distance_sq < kAcquisitionRangeSq) {
			target_pos_ = target == 1 ? p1_pos : p2_pos;
			state_ = ai::State::kChase;
		}
		else if (distance_sq < kAIActivationRangeSq){
			float rad = rng.Rand(0.0f, 2 * acosf(-1));
			target_pos_.x = GetPostion().x + 200.0f * cosf(rad);
			target_pos_.y = GetPostion().y + 200.0f * sinf(rad);
			state_ = ai::State::kWander;
		}
		else {
			state_ = ai::State::kNoAI;
		}
	}

	void Monster::Act() noexcept
	{
		switch (state_)
		{
		case ai::State::kNoAI:
			break;
		case ai::State::kWander:
			Move();
			break;
		case ai::State::kChase:
			Move();
			break;
		case ai::State::kAttack:
			break;
		default:
			break;
		}
	}

	void Monster::Move() noexcept
	{
		
	}

	void Monster::Attack() noexcept
	{
		
	}
}