//---------------------------------------------------
// 
// cas_lock.h - CASLock 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <atomic>

namespace lf {
	// TryLock만을 제공: 블로킹 알고리즘에 사용하지 아니한다.
	class CASLock {
	public:
		CASLock() noexcept : has_locked{} {}

		bool TryLock() noexcept {
			if (has_locked) {
				return false;
			}
			return CAS(&has_locked, false, true);
		}

		void Unlock() noexcept {
			has_locked = false;
		}
	private:
		bool CAS(volatile bool* value, bool expected, bool desired) noexcept {
			return std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_bool*>(value), &expected, desired);
		}

		volatile bool has_locked;
	};
}
