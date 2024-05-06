//---------------------------------------------------
// 
// thread.h - ������ ���� ���� �Լ�/���/���� ����
// 
//---------------------------------------------------

#pragma once
#include <thread>

namespace thread {
	constexpr inline int kUnregisteredID = -1;

	inline const int kNumWorkers = static_cast<int>(std::thread::hardware_concurrency()) * 2;
	int ID(int id_to_register = kUnregisteredID) noexcept;
}