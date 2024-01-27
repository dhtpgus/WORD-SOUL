#include <atomic>
#include "thread.h"

int thread::GetNumWorker() {
	return 2 * (int)std::thread::hardware_concurrency();
}

int thread::GetID() {
	static std::atomic<int> cnt;
	static thread_local int id = cnt.fetch_add(1);
	return id;
}