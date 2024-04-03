//---------------------------------------------------
// 
// timer.h - Timer 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <chrono>

class Timer {
public:
	using Duration = double;
	Timer() noexcept : time_point_{ Clock::now() }, accumulated_duration{} {}
	Duration GetDuration() noexcept {
		auto duration = Clock::now() - time_point_;
		Duration sec = duration_cast<std::chrono::microseconds>(duration).count() / 1e6;
		accumulated_duration += sec;
		ResetTimePoint();
		return sec;
	}
	void ResetAccumulatedDuration() noexcept {
		accumulated_duration = 0;
	}
	Duration GetAccumulatedDuration() noexcept {
		return accumulated_duration;
	}
private:
	void ResetTimePoint() noexcept {
		time_point_ = Clock::now();
	}
	using Clock = std::chrono::high_resolution_clock;
	Clock::time_point time_point_;
	Duration accumulated_duration;
};