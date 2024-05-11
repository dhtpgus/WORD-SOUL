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
}