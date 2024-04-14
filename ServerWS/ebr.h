//---------------------------------------------------
// 
// ebr.h - ebr 메모리 관리 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <queue>
#include <vector>
#include <atomic>
#include <iostream>
#include "thread.h"
#include "lf_node.h"

#undef max
#undef min

namespace lf {
	template<class T>
	class EBR {
	public:
		EBR() = delete;
		EBR(int thread_num) noexcept : reservations_{ new RetiredEpoch[thread_num] },
			thread_num_{ thread_num }, epoch_{} {
			for (int i = 0; i < thread_num; ++i) {
				reservations_[i] = std::numeric_limits<RetiredEpoch>::max();
				retired_.emplace_back();
			}
		}
		~EBR() noexcept {
			for (auto& q : retired_) {
				while (not q.empty()) {
					auto f = q.front();
					q.pop();
					free_list<T>.Collect(f);
				}
			}
			delete[] reservations_;
		}
		EBR(const EBR&) = delete;
		EBR(EBR&&) = delete;
		EBR& operator=(const EBR&) = delete;
		EBR& operator=(EBR&&) = delete;
		void Retire(T* ptr) noexcept {
			ptr->retire_epoch = epoch_.load(std::memory_order_relaxed);
			retired_[thread::ID()].push(ptr);
			if (retired_[thread::ID()].size() >= GetCapacity()) {
				Clear();
			}
		}
		void StartOp() noexcept {
			reservations_[thread::ID()] = epoch_.fetch_add(1, std::memory_order_relaxed);
		}
		void EndOp() noexcept {
			reservations_[thread::ID()] = std::numeric_limits<RetiredEpoch>::max();
		}

	private:
		using RetiredEpoch = unsigned long long;
		using RetiredNodeQueue = std::queue<T*>;

		constexpr RetiredEpoch GetCapacity() const noexcept {
			return (RetiredEpoch)(3 * thread_num_ * 2 * 10);
		}

		RetiredEpoch GetMinReservation() const noexcept {
			RetiredEpoch min_re = std::numeric_limits<RetiredEpoch>::max();
			for (int i = 0; i < thread_num_; ++i) {
				min_re = std::min(min_re, (RetiredEpoch)reservations_[i]);
			}
			return min_re;
		}
		void Clear() noexcept {
			RetiredEpoch max_safe_epoch = GetMinReservation();

			while (false == retired_[thread::ID()].empty()) {
				auto f = retired_[thread::ID()].front();
				if (f->retire_epoch >= max_safe_epoch)
					break;
				retired_[thread::ID()].pop();

				free_list<T>.Collect(f);
			}
		}

		RetiredEpoch* volatile reservations_;
		std::vector<RetiredNodeQueue> retired_;
		int thread_num_;
		std::atomic<RetiredEpoch> epoch_;
	};
}