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

		lua::Script server_settings{ "scripts/server_settings.lua" };
		displays_msg = server_settings.GetConstant<bool>("DISPLAYS_MSG");

		has_read = true;

		return displays_msg;
	};
#endif
}