//---------------------------------------------------
// 
// lf_array.h - lf::Array 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <numeric>
#include "session.h"
#include "cas_lock.h"
#include "debug.h"
#include "random_number_generator.h"

namespace lf {
	namespace array {
		using ID = unsigned short;

		template<class T>
		struct Element {
			Element() noexcept = default;
			~Element() noexcept {
				if (ref_cnt > 0) {
					delete data;
				}
			}
			Element(const Element&) = delete;
			Element(Element&&) = delete;
			Element& operator=(const Element&) = delete;
			Element& operator=(Element&&) = delete;

			static constexpr auto kDeleted{ -1 };
			T* data{};
			std::atomic_int ref_cnt{ kDeleted };
			CASLock cas_lock{};
			volatile bool is_deleted{};
		};

		struct ElementID {
			void Reset(ID rs_id) {
				id = rs_id;
			}
			ID id;
		};
	}

	template<class T>
	class Array {
	public:
		using ID = array::ID;
		Array() = delete;
		Array(int el_num, int th_num) noexcept : elements_(el_num), id_queue_{ th_num } {
			std::vector<ID> indexes(el_num);
			std::iota(indexes.begin(), indexes.end(), 0);
			std::shuffle(indexes.begin(), indexes.end(), std::mt19937{ std::random_device{}() });

			std::vector<std::thread> threads;
			for (int i = 0; i < th_num; ++i) {
				threads.emplace_back([i, el_num, th_num, indexes, this]() {
					thread::ID(i);
					for (int j = i; j < el_num; j += th_num) {
						id_queue_.Emplace<array::ElementID>(indexes[j]);
					}
					});
			}
			for (auto& th : threads) {
				th.join();
			}
		}
		// 접근 전 TryAccess 메소드를 먼저 실행하여야 한다.
		// 사용이 끝나면 EndAccess를 실행하여야 한다.
		T& operator[](ID index) noexcept {
			return *elements_[index].data;
		}
		T& Get(ID index) noexcept {
			return *elements_[index].data;
		}
		bool TryAccess(ID index) noexcept {
			if ((not IsIDValid(index)) or elements_[index].is_deleted) {
				return false;
			}
			while (true) {
				int ref_cnt = elements_[index].ref_cnt.load();
				if (ref_cnt <= 0) {
					return false;
				}
				if (CAS(elements_[index].ref_cnt, ref_cnt, ref_cnt + 1)) {
					return true;
				}
			}
		}
		void EndAccess(ID index) noexcept {
			if (not IsIDValid(index)) {
				return;
			}
			elements_[index].ref_cnt -= 1;
			TryDelete(index);
		}
		void ReserveDelete(ID index) noexcept {
			if (not IsIDValid(index)) {
				return;
			}
			if (not elements_[index].cas_lock.TryLock()) {
				return;
			}
			elements_[index].is_deleted = true;
			elements_[index].ref_cnt -= 1;
			TryDelete(index);
		}
		template<class Type, class... Value>
		int Allocate(Value&&... value) noexcept {
			auto pop = id_queue_.Pop();
			if (nullptr == pop) {
				if (debug::DisplaysMSG()) {
					std::print("[Warning] Failed to Allocate: Capacity Exceeded\n");
				}
				return kInvalidID;
			}
			ID id = pop->id;
			free_list<array::ElementID>.Collect(pop);

			if (false == elements_[id].is_deleted) {
				elements_[id].data = new Type{ id, value... };
			}
			else {
				elements_[id].data->Reset(id, value...);
			}
			elements_[id].cas_lock.Unlock();
			elements_[id].ref_cnt = 1;
			elements_[id].is_deleted = false;
			return id;
		}
		bool Exists(ID id) const noexcept {
			return IsIDValid(id) and (not elements_[id].is_deleted) and elements_[id].ref_cnt > 0;
		}
		static constexpr int kInvalidID = -1;
	private:
		bool CAS(std::atomic_int& mem, int expected, int desired) noexcept {
			return mem.compare_exchange_strong(expected, desired);
		}
		void TryDelete(ID index) noexcept {
			if (CAS(elements_[index].ref_cnt, 0, array::Element<T>::kDeleted)) {
				elements_[index].data->Delete();
				id_queue_.Emplace<array::ElementID>(index);
			}
		}
		bool IsIDValid(ID index) const noexcept {
			if (0 <= index and index < elements_.size()) {
				return true;
			}
			return false;
		}
		std::vector<array::Element<T>> elements_;
		RelaxedQueue<array::ElementID, 1e9> id_queue_;
	};
}