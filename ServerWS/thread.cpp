#include <atomic>
#include <print>
#include "thread.h"

int thread::GetNumWorker() noexcept
{
	return (int)std::thread::hardware_concurrency();
}

int thread::ID(int id_to_register) noexcept
{
	static thread_local const int id{ id_to_register };
	if (id == kUnregisteredID) {
		std::print("[Error] Must Register Thread ID First.\n");
		system("pause");
		exit(1);
	}
	if (id_to_register != kUnregisteredID and id != id_to_register) {
		std::print("[Error] Thread already has an ID.\n");
		system("pause");
		exit(1);
	}
	return id;
}