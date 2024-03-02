#include <atomic>
#include <print>
#include "thread.h"

int thread::GetNumWorker()
{
	return 2 * (int)std::thread::hardware_concurrency();
}

int thread::ID(int id_to_register)
{
	static thread_local int id = id_to_register;
	if (id == kUnregisteredID) {
		std::print("[오류] 스레드 아이디를 먼저 등록해야 합니다.\n");
		system("pause");
		exit(1);
	}
	if (id_to_register != kUnregisteredID and id != id_to_register) {
		std::print("[오류] 스레드 아이디가 이미 등록되었습니다.\n");
		system("pause");
		exit(1);
	}
	return id;
}