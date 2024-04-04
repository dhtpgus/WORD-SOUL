#include <print>
#include <fstream>
#include "debug.h"

#define IS_RELEASE_MODE 0

namespace debug {
#if IS_RELEASE_MODE
	constexpr bool DisplaysMSG() noexcept {
		return false;
	}
#else
	bool DisplaysMSG() noexcept {
		static bool displays_msg;
		static bool has_read_file;

		if (has_read_file) {
			return displays_msg;
		}

		std::string file_name{ "displays_msg.txt" };

		std::ifstream in{ std::format("data/{}", file_name) };
		if (not in) {
			in.open(std::format("../../data/{}", file_name));
			if (not in) {
				std::print("[Error] Cannot Open file: data/{}\n", file_name);
				exit(1);
			}
		}
		in >> displays_msg;
		has_read_file = true;

		return displays_msg;
	};
#endif
}