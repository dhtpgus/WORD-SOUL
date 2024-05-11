#include <print>
#include <fstream>
#include "debug.h"
#include "lua_script.h"

namespace debug {
#if IS_RELEASE_MODE
	constexpr bool DisplaysMSG() noexcept {
		return false;
	}
#else
	bool DisplaysMSG() noexcept {
		static bool displays_msg;
		static bool has_read;

		if (has_read) {
			return displays_msg;
		}

		displays_msg = static_cast<bool>(lua::server_data.GetGlobalVar<int>("displays_msg"));

		has_read = true;

		return displays_msg;
	};
#endif
}