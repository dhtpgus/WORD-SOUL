//---------------------------------------------------
// 
// monster.h - 몬스터 엔티티 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include "entity.h"
#include "fsm.h"

namespace entity {
	class Monster : public Base {
	public:
		Monster(ID id, float x, float y, float z, short hp) noexcept
			: Base{ id, x, y, z, hp }, state_{ ai::State::kNoAI }, target_pos_{} {}
		void Decide(const Position& p1_pos, const Position& p2_pos) noexcept;
		void Act() noexcept;
	private:
		void Move() noexcept;
		void Attack() noexcept;
		constexpr static float kAcquisitionRange = 1000.0f;
		constexpr static float kAcquisitionRangeSq = kAcquisitionRange * kAcquisitionRange;
		constexpr static float kAttackRange = 30.0f;
		constexpr static float kAttackRangeSq = kAttackRange * kAttackRange;
		constexpr static float kAIActivationRange = 3000.0f;
		constexpr static float kAIActivationRangeSq = kAIActivationRange * kAIActivationRange;
		ai::State state_;
		Position target_pos_;
	};
}