//---------------------------------------------------
// 
// thread.h - ������ ���� ���� �Լ�/���/���� ����
// 
//---------------------------------------------------

#pragma once
#include <thread>

namespace thread {
	constexpr inline int kUnregisteredID = -1;

	int GetNumWorker() noexcept;
	int ID(int id_to_register = kUnregisteredID) noexcept;
}