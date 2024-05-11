#pragma once

namespace fsm {
	enum class State : char {
		kAIDisabled, kWander, kChase, kAttack
	};
}