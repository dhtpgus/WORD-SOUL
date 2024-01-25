//---------------------------------------------------
// 
// thread.h - 스레드 관련 전역 함수/상수/변수 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>

namespace thread {
	int GetNumWorker() {
		return 2 * (int)std::thread::hardware_concurrency();
	}

	int GetID() {
		static int cnt;
		static thread_local int id = cnt++;
		return id;
	}
}