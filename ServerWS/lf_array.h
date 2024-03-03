//---------------------------------------------------
// 
// lf_array.h - lf::Array 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include "client_socket.h"
#include "cas_lock.h"

namespace lf {
	template<class T>
	struct alignas(std::hardware_destructive_interference_size) Element {
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

		std::atomic_int ref_cnt{ kDeleted };
		CASLock cas_lock{};
		T* data{};

		static constexpr auto kDeleted{ -1 };
	};

	template<class T>
	class Array {
	public:
		Array() = delete;
		Array(int el_num, int th_num) : elements_(el_num), index_queue_{ th_num } {
			for (int i = 0; i < el_num; ++i) {
				index_queue_.Emplace(i);
			}
		}
		T& operator[](int i) {
			return *elements_[i].data;
		}
		bool StartAccess(int i) {
			if (not IsIndexValid(i)) {
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
			if (not IsIndexValid(i)) {
				return;
			}
			elements_[i].ref_cnt -= 1;
			TryDelete(i);
		}
		void ReserveDelete(int i) {
			if (not IsIndexValid(i)) {
				return;
			}
			if (not elements_[i].cas_lock.TryLock()) {
				return;
			}
			elements_[i].ref_cnt -= 1;
			TryDelete(i);
		}
		template<class... Value>
		int Allocate(Value&&... value) {
			auto pop = index_queue_.Pop();
			if (nullptr == pop) {
				std::print("[경고] 할당 실패 - 허용량 초과\n");
				return kAllocationFailed;
			}
			int id = *pop;
			delete pop;

			elements_[id].data = new T{ id, value... };
			elements_[id].cas_lock.Unlock();
			elements_[id].ref_cnt = 1;
			return id;
		}
		bool Exists(int id) const {
			return IsIndexValid(id) and elements_[id].ref_cnt > 0;
		}
		int Count() const {
			int cnt = 0;
			for (const auto& e : elements_) {
				cnt += (e.ref_cnt > 0 ? 1 : 0);
			}
			return cnt;
		}
		static constexpr int kAllocationFailed = -1;
	private:
		bool CAS(std::atomic_int& mem, int expected, int desired) {
			return mem.compare_exchange_strong(expected, desired);
		}
		void TryDelete(int i) {
			if (CAS(elements_[i].ref_cnt, 0, Element<T>::kDeleted)) {
				delete elements_[i].data;
				index_queue_.Emplace(i);
			}
		}
		bool IsIndexValid(int i) const {
			if (0 <= i and i < elements_.size()) {
				return true;
			}
			return false;
		}
		std::vector<Element<T>> elements_;
		RelaxedQueue<int> index_queue_;
	};
}