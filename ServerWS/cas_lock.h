//---------------------------------------------------
// 
// cas_lock.h - CASLock 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <atomic>

namespace lf {
	// TryLock만을 제공: 블로킹 알고리즘에 사용하지 않도록 주의한다.
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

	class CASLockGuard {
	public:
		CASLockGuard() = delete;
		CASLockGuard(CASLock& cas_lock) noexcept : cas_lock_{ &cas_lock } {
			auto msb = kMSBMask * static_cast<size_t>(!cas_lock_->TryLock());
			auto qword = reinterpret_cast<size_t>(cas_lock_);
			cas_lock_ = reinterpret_cast<CASLock*>(qword | msb);
		};
		CASLockGuard(const CASLockGuard&) = delete;
		CASLockGuard(CASLockGuard&&) = delete;
		CASLockGuard& operator=(const CASLockGuard&) = delete;
		CASLockGuard& operator=(CASLockGuard&&) = delete;
		~CASLockGuard() noexcept {
			if (*this) {
				cas_lock_->Unlock();
			}
		}
		operator bool() const noexcept {
			return 0 == (reinterpret_cast<size_t>(cas_lock_) & kMSBMask);
		}
	private:
		static constexpr auto kMSBMask{ 0x8000'0000'0000'0000 };
		CASLock* cas_lock_;
	};
}
