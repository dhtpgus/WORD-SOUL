//---------------------------------------------------
// 
// thread.h - 스레드 관련 전역 함수/상수/변수 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>

namespace thread {
	static const int kNumWorker{ 2 * (int)std::thread::hardware_concurrency() };
	static thread_local int id;
}