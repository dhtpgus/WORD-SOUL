#include <print>
#include "monster.h"
#include "boss.h"
#include "player.h"

namespace entity {
	char Base::GetFlag() const noexcept
	{
		char flag{};
		auto type = GetType();

		switch (type) {
		case Type::kPlayer:
		{
			break;
		}
		case Type::kMonster:
		{
			auto m = reinterpret_cast<const Monster*>(this);
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
		case Type::kMonster:
		{
			auto m = reinterpret_cast<const Monster*>(this);
			switch (m->GetState()) {
			case fsm::State::kChase:
				return m->kChaseSpeed;
			case fsm::State::kWander:
				return m->kWanderSpeed;
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