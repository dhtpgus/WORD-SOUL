//---------------------------------------------------
// 
// timer.h - Timer 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <chrono>

class Timer {
public:
	using Duration = float;
	Timer() noexcept : time_point_{ Clock::now() } {}
	Duration GetDuration() noexcept {
		auto duration = Clock::now() - time_point_;
		Duration sec = duration_cast<std::chrono::microseconds>(duration).count() / 1e6f;
		ResetTimePoint();
		return sec;
	}
private:
	void ResetTimePoint() noexcept {
		time_point_ = Clock::now();
	}
	using Clock = std::chrono::high_resolution_clock;
	Clock::time_point time_point_;
};