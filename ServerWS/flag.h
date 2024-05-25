#pragma once

namespace flag {
	struct Base {

	};

	struct Bit8 : Base {
		char data;
	};

	struct Bit16 : Base {
		short data;
	};

	struct Bit32 : Base {
		int data;
	};

	struct Bit64 : Base {
		long long data;
	};
}