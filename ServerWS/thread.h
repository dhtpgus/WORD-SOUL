//---------------------------------------------------
// 
// thread.h - 스레드 관련 전역 함수/상수/변수 정의
// 
//---------------------------------------------------

#pragma once
#include <thread>

namespace thread {
	int GetNumWorker();
	int GetID();
}