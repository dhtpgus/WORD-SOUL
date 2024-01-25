//---------------------------------------------------
// 
// thread.h - ������ ���� ���� �Լ�/���/���� ����
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