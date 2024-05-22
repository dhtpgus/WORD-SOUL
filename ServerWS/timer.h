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
	Duration GetDuration(float time) noexcept {
		auto duration = Clock::now() - time_point_;
		Duration sec = duration_cast<std::chrono::microseconds>(duration).count() / 1e6f;
		if (time > sec) {
			return 0.0f;
		}
		ResetTimePoint();
		return sec;
	}
	void ResetTimePoint() noexcept {
		time_point_ = Clock::now();
	}
private:
	using Clock = std::chrono::high_resolution_clock;
	Clock::time_point time_point_;
};