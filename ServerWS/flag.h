#pragma once
#include <atomic>

template<class T>
struct Flag {
	bool SetBit(T i) {
		auto local_data = data;
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<T>*>(&data),
			&local_data, local_data | i);
	}
	bool ClearBit(T i) {
		auto local_data = data;
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<T>*>(&data),
			&local_data, local_data & (~i));
	}
	bool ToggleBit(T i) {
		auto local_data = data;
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<T>*>(&data),
			&local_data, local_data ^ (~i));
	}
	bool Set(T expected, T desired) {
		return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<T>*>(&data),
			&expected, desired);
	}

	volatile T data;
};