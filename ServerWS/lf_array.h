#pragma once
#include "client_socket.h"
#include "cas_lock.h"

namespace lf {
	template<class T>
	struct alignas(std::hardware_destructive_interference_size) Element {
		std::atomic_int cnt{};
		CASLock cas_lock{};
		T* data{};
		Element() = default;
		~Element() {
			if (cnt > 0) {
				delete data;
			}
		}
	};

	template<class T>
	class Array {
	public:
		Array() = delete;
		Array(int th_num) : elements_(0), index_queue_{ th_num } {}
		Array(int el_num, int th_num) : elements_(el_num), index_queue_{ th_num } {}
		void InitIndexes(int el_num) {
			for (int i = 0; i < el_num; ++i) {
				index_queue_.Emplace(i);
			}
		}
		void InitIndexes() {
			InitIndexes(elements_.size());
		}
		T& operator[](int i) {
			return *elements_[i].data;
		}
		bool StartAccess(int i) {
			if (not IsIndexValid(i)) {
				return false;
			}
			while (true) {
				int cnt = elements_[i].cnt.load();
				if (cnt <= 0) {
					return false;
				}
				if (CAS(elements_[i].cnt, cnt, cnt + 1)) {
					return true;
				}
			}
		}
		void EndAccess(int i) {
			if (not IsIndexValid(i)) {
				return;
			}
			elements_[i].cnt -= 1;
			TryDelete(i);
		}
		void ReserveDelete(int i) {
			if (not IsIndexValid(i)) {
				return;
			}
			if (not elements_[i].cas_lock.TryLock()) {
				return;
			}
			elements_[i].cnt -= 1;
			TryDelete(i);
		}
		template<class... Value>
		int Allocate(Value&&... value) {
			auto pop = index_queue_.Pop();
			if (nullptr == pop) {
				std::print("null\n");
				return -1;
			}
			int id = *pop;
			delete pop;

			elements_[id].data = new T{ id, value... };
			elements_[id].cas_lock.Unlock();
			elements_[id].cnt = 1;
			return id;
		}
		bool Exists(int id) const {
			return IsIndexValid(id) and elements_[id].cnt > 0;
		}
		int Count() {
			int cnt = 0;
			for (const auto& e : elements_) {
				cnt += (e.cnt > 0 ? 1 : 0);
			}
			return cnt;
		}
	private:
		bool CAS(std::atomic_int& mem, int expected, int desired) {
			return mem.compare_exchange_strong(expected, desired);
		}
		void TryDelete(int i) {
			if (CAS(elements_[i].cnt, 0, -1)) {
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