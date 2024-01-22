//---------------------------------------------------
// 
// thread.h - ������ ���� ���� �Լ�/���/���� ����
// 
//---------------------------------------------------

#pragma once
#include <thread>

namespace thread {
	static const int kNumWorker{ 2 * (int)std::thread::hardware_concurrency() };
	static thread_local int id;
}