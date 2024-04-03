//---------------------------------------------------
// 
// lf_array.h - lf::Array Ŭ���� ����
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
		struct ElementID {
			void Reset(ID rs_id) {
				id = rs_id;
			}
			ID id;
		};

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

			T* data{};
			std::atomic_int ref_cnt{ kDeleted };
			CASLock cas_lock{};
			volatile bool is_deleted{};

			static constexpr auto kDeleted{ -1 };
		};
	}

	template<class T>
	class Array {
	public:
		using ID = array::ID;
		Array() = delete;
		Array(int el_num, int th_num) noexcept : elements_(el_num), id_queue_{ th_num } {
			std::vector<unsigned short> indexes(el_num);
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
		// ���� �� TryAccess �޼ҵ带 ���� �����Ͽ��� �Ѵ�.
		// ����� ������ EndAccess�� �����Ͽ��� �Ѵ�.
		T& operator[](ID index) noexcept {
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
				if (debug::IsDebugMode()) {
					std::print("[Warning] Failed to Allocate: Capacity Exceeded\n");
				}
				return kInvalidID;
			}
			ID id = pop->id;
			delete pop;

			elements_[id].data = new Type{ id, value... };
			elements_[id].cas_lock.Unlock();
			elements_[id].is_deleted = false;
			elements_[id].ref_cnt = 1;
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
				delete elements_[index].data;
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
		RelaxedQueue<array::ElementID> id_queue_;
	};
}