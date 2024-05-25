#include <print>
#include "mob.h"
#include "boss.h"
#include "player.h"

namespace entity {
	char Base::GetFlag() const noexcept
	{
		char flag{ flag_ };
		auto type = GetType();

		switch (type) {
		case Type::kPlayer:
		{
			break;
		}
		case Type::kMob:
		{
			auto m = reinterpret_cast<const Mob*>(this);
			flag |= static_cast<char>(m->GetState());
			break;
		}
		case Type::kBoss:
		{
			break;
		}
		default:
		{
			std::print("[Error] Unknown Entity Type: {}\n", static_cast<int>(type));
			exit(1);
		}
		}

		return flag;
	}

	float Base::GetVel() const noexcept
	{
		auto type = GetType();
		switch (type) {
		case Type::kPlayer:
			return 600.0f;
		case Type::kMob:
		{
			auto m = reinterpret_cast<const Mob*>(this);
			switch (m->GetState()) {
			case fsm::State::kChase:
				return mob::vel_chase;
			case fsm::State::kWander:
				return mob::vel_wander;
			default:
				return 0.0f;
			}
		}
		case Type::kBoss:
		{
			break;
		}
		default:
		{
			std::print("[Error] Unknown Entity Type: {}\n", static_cast<int>(type));
			exit(1);
		}
		}
		return 0.0f;
	}
}