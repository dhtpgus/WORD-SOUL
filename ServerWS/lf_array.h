//---------------------------------------------------
// 
// lf_array.h - lf::Array 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include "client_socket.h"
#include "cas_lock.h"
#include "debug.h"

namespace lf {
	template<class T>
	struct Element {
		Element() = default;
		~Element() {
			if (ref_cnt > 0) {
				delete data;
			}
		}
		Element(const Element&) = delete;
		Element(Element&&) = delete;
		Element& operator=(const Element&) = delete;
		Element& operator=(Element&&) = delete;

		T* data{};
		CASLock cas_lock{};
		std::atomic_int ref_cnt{ kDeleted };

		static constexpr auto kDeleted{ -1 };
	};

	template<class T>
	class Array {
	public:
		Array() = delete;
		Array(int el_num, int th_num) : elements_(el_num), index_queue_{ th_num } {
			std::vector<std::thread> threads;
			for (int i = 0; i < th_num; ++i) {
				threads.emplace_back([i, el_num, th_num, this]() {
					thread::ID(i);
					for (int j = i; j < el_num; j += th_num) {
						index_queue_.Emplace<int>(j);
					}
					});
			}
			for (auto& th : threads) {
				th.join();
			}
		}
		// 접근 전 TryAccess 메소드를 먼저 실행하여야 한다.
		// 사용이 끝나면 EndAccess를 실행하여야 한다.
		T& operator[](int i) {
			return *elements_[i].data;
		}
		bool TryAccess(int i) {
			if (not IsIDValid(i)) {
				return false;
			}
			while (true) {
				int ref_cnt = elements_[i].ref_cnt.load();
				if (ref_cnt <= 0) {
					return false;
				}
				if (CAS(elements_[i].ref_cnt, ref_cnt, ref_cnt + 1)) {
					return true;
				}
			}
		}
		void EndAccess(int i) {
			if (not IsIDValid(i)) {
				return;
			}
			elements_[i].ref_cnt -= 1;
			TryDelete(i);
		}
		void ReserveDelete(int i) {
			if (not IsIDValid(i)) {
				return;
			}
			if (not elements_[i].cas_lock.TryLock()) {
				return;
			}
			elements_[i].ref_cnt -= 1;
			TryDelete(i);
		}
		template<class Type, class... Value>
		int Allocate(Value&&... value) {
			auto pop = index_queue_.Pop();
			if (nullptr == pop) {
				if (debug::IsDebugMode()) {
					std::print("[Warning] Failed to Allocate: Capacity Exceeded\n");
				}
				return kInvalidID;
			}
			int id = *pop;
			delete pop;

			elements_[id].data = new Type{ id, value... };
			elements_[id].cas_lock.Unlock();
			elements_[id].ref_cnt = 1;
			return id;
		}
		bool Exists(int id) const {
			return IsIDValid(id) and elements_[id].ref_cnt > 0;
		}
		int Count() const {
			int cnt = 0;
			for (const auto& e : elements_) {
				cnt += (e.ref_cnt > 0 ? 1 : 0);
			}
			return cnt;
		}
		static constexpr int kInvalidID = -1;
	private:
		bool CAS(std::atomic_int& mem, int expected, int desired) {
			return mem.compare_exchange_strong(expected, desired);
		}
		void TryDelete(int i) {
			if (CAS(elements_[i].ref_cnt, 0, Element<T>::kDeleted)) {
				/*if (false and elements_[i].data->is_dangerous_to_delete) {
					elements_[i].data->DeleteLogically();
				}
				else {*/
				delete elements_[i].data;
				//}
				index_queue_.Emplace<int>(i);
			}
		}
		bool IsIDValid(int i) const {
			if (0 <= i and i < elements_.size()) {
				return true;
			}
			return false;
		}
		std::vector<Element<T>> elements_;
		RelaxedQueue<int> index_queue_;
	};
}