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
		std::print("[Error] Must Register Thread ID First.\n");
		system("pause");
		exit(1);
	}
	if (id_to_register != kUnregisteredID and id != id_to_register) {
		std::print("[Error] Thread ID has already registered\n");
		system("pause");
		exit(1);
	}
	return id;
}