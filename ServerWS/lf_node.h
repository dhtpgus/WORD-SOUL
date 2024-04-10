//---------------------------------------------------
// 
// lf_node.h - lf::Node 구조체 정의
// 
//---------------------------------------------------

#pragma once
#include <chrono>

namespace lf {
	struct Node {
		using Value = void*;
		using Level = unsigned long long;
		using RetireEpoch = unsigned long long;
		using Duration = long long;
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = Clock::time_point;

		Node* volatile next;
		Value v;
		Level level;
		RetireEpoch retire_epoch;
		Duration duration;
		int registrant;

		Node(Value v, Level level, TimePoint tp, int registrant) noexcept
			: v{ v }, level{ level }, next{ nullptr }, retire_epoch{}
			, duration{ (std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - tp)).count() }
			, registrant{ registrant } {}
		void Reset(Value rs_v, Level rs_level, TimePoint tp, int rs_registrant) {
			v = rs_v;
			level = rs_level;
			next = nullptr;
			retire_epoch = 0;
			registrant = rs_registrant;
			duration = (std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - tp)).count();
		}
	};
}