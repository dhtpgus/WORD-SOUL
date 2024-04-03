#pragma once
#include "monster.h"

namespace ai {
	class FSM {
	public:
		virtual void Enter(entity::Monster& monster) noexcept = 0;
		virtual void Execute(entity::Monster& monster) noexcept = 0;
		virtual void Exit(entity::Monster& monster) noexcept = 0;
	};
}