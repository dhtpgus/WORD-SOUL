#include <atomic>
#include <print>
#include "thread.h"
#include "lua_script.h"
#include "debug.h"

namespace thread {
	int GetNumWorkers() noexcept
	{
		static int num_workers;
		static bool has_read;

		if (has_read) {
			return num_workers;
		}

		lua::Script server_settings{ "scripts/server_settings.lua" };

		num_workers = server_settings.GetConstant<int>("WORKERS");
		if (num_workers == 0) {
			num_workers = static_cast<int>(std::thread::hardware_concurrency() * 2);
		}

		if (debug::DisplaysMSG()) {
			std::print("[Info] Worker Threads: {}\n", num_workers);
		}
		has_read = true;

		return num_workers;
	}

	int ID(int id_to_register) noexcept
	{
		static thread_local const int kID{ id_to_register };
		if (debug::DisplaysMSG()) {
			if (kID == kUnregisteredID) {
				std::print("[Error] Must Register Thread ID First.\n");
				system("pause");
				exit(1);
			}
			if (id_to_register != kUnregisteredID and kID != id_to_register) {
				std::print("[Error] Thread already has an ID.\n");
				system("pause");
				exit(1);
			}
		}
		return kID;
	}
}