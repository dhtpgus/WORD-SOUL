#pragma once
#include <thread>
#include "over_ex.h"

inline void TimerThread()
{
	constexpr auto kDelay = 1000'000;

	while (true) {
		std::this_thread::sleep_for( std::chrono::milliseconds{ kDelay } );
	}
}