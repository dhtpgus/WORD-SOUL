//---------------------------------------------------
// 
// ebr.h - ebr 메모리 관리자 클래스 정의
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
		EBR(int thread_num) : reservations_{ new Epoch[thread_num] },
			thread_num_{ thread_num }, epoch_{} {
			for (int i = 0; i < thread_num; ++i) {
				reservations_[i] = std::numeric_limits<Epoch>::max();
				retired_.emplace_back();
			}
		}
		~EBR() {
			delete[] reservations_;
		}
		EBR(const EBR&) = delete;
		EBR(EBR&&) = delete;
		EBR& operator=(const EBR&) = delete;
		EBR& operator=(EBR&&) = delete;
		void Retire(T* ptr) {
			ptr->retire_epoch = epoch_.load(std::memory_order_relaxed);
			retired_[thread::GetID()].push(ptr);
			if (retired_[thread::GetID()].size() >= GetCapacity()) {
				Clear();
			}
		}
		void StartOp() {
			reservations_[thread::GetID()] = epoch_.fetch_add(1, std::memory_order_relaxed);
		}
		void EndOp() {
			reservations_[thread::GetID()] = std::numeric_limits<Epoch>::max();
		}

	private:
		using Epoch = unsigned long long;
		using RetiredNodeQueue = std::queue<T*>;

		constexpr Epoch GetCapacity() const {
			return (Epoch)(3 * thread_num_ * 2 * 10);
		}

		Epoch GetMinReservation() const {
			Epoch min_re = std::numeric_limits<Epoch>::max();
			for (int i = 0; i < thread_num_; ++i) {
				min_re = std::min(min_re, (Epoch)reservations_[i]);
			}
			return min_re;
		}
		void Clear() {
			Epoch max_safe_epoch = GetMinReservation();

			while (false == retired_[thread::GetID()].empty()) {
				auto f = retired_[thread::GetID()].front();
				if (f->retire_epoch >= max_safe_epoch)
					break;
				retired_[thread::GetID()].pop();

				delete f;
			}
		}

		Epoch* volatile reservations_;
		std::vector<RetiredNodeQueue> retired_;
		int thread_num_;
		std::atomic<Epoch> epoch_;
	};
}