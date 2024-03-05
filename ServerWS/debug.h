#pragma once
#include <print>
#include <fstream>

namespace debug {
	static bool IsDebugMode() {
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
		if (is_debug_mode) {
			std::print("[Info] Started in Debug Mode\n");
		}
		has_read_file = true;

		return is_debug_mode;
	};
}