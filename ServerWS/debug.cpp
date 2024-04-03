#include <print>
#include <fstream>
#include "debug.h"

namespace debug {
	bool IsDebugMode() noexcept {
		static bool is_debug_mode;
		static bool has_read_file;

		if (has_read_file) {
			return is_debug_mode;
		}
		std::ifstream in{ "data/is_debug_mode.txt" };
		if (not in) {
			in.open("../../data/is_debug_mode.txt");
			if (not in) {
				std::print("[Error] Cannot Open file: data/is_debug_mode.txt");
				exit(1);
			}
		}
		in >> is_debug_mode;
		has_read_file = true;

		return is_debug_mode;
	};
}