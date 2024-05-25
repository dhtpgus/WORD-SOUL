#pragma once

struct Flag {
	using T = char;

	void SetBit(int i) {
		data |= static_cast<T>(1ULL << i);
	}
	void ClearBit(int i) {
		data &= static_cast<T>(~(1ULL << i));
	}
	void ToggleBit(int i) {
		data ^= static_cast<T>(~(1ULL << i));
	}

	T data;
};