#include <print>
#include <fstream>
#include "debug.h"
#include "lua_script.h"

namespace debug {
#if not IS_RELEASE_MODE
	bool DisplaysMSG() noexcept {
		static bool displays_msg;
		static bool has_read;

		if (has_read) {
			return displays_msg;
		}

		displays_msg = lua::server_settings.GetGlobalVar<bool>("displays_msg");

		has_read = true;

		return displays_msg;
	};
#endif
}