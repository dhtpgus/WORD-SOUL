#pragma once
#include <queue>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include "lf_array.h"
#include "lf_base15_tree.h"
#include "free_list.h"

#define USES_NONBLOCKING_DS 01

namespace concurrent {
	namespace blocking {
		template<class T, double>
		class Queue {
		public:
			Queue() = default;
			Queue(int) {};
			T* Pop() {
				T* r{};
				mx_.lock();
				if (not queue_.empty()) {
					r = queue_.front();
					queue_.pop();
				}
				mx_.unlock();
				return r;
			}
			template<class Type, class... Value>
			void Emplace(Value&&... value) noexcept {
				auto e = reinterpret_cast<T*>(free_list<Type>.Get(value...));
				mx_.lock();
				queue_.push(e);
				mx_.unlock();
			}
		private:
			std::mutex mx_;
			std::queue<T*> queue_;
		};

		template<class T>
		class Array {
		public:
			using ID = unsigned short;
			Array(int el_num, int) : kMaxElement{ el_num } {
				data_ = new T*[el_num]{};
				mx_ = new std::mutex[el_num]{};

				std::vector<int> indexes(el_num);
				std::iota(indexes.begin(), indexes.end(), 0);
				std::shuffle(indexes.begin(), indexes.end(), std::mt19937{ std::random_device{}() });

				for (int i : indexes) {
					queue_.push(i);
				}
			}
			~Array() {
				delete[] data_;
				delete[] mx_;
			}
			template<class Type, class... Value>
			int Allocate(Value&&... value) noexcept {
				q_mx_.lock();
				if (not queue_.empty()) {
					ID id = queue_.front();
					queue_.pop();
					q_mx_.unlock();
					data_[id] = new Type{ id, value... };
					return id;
				}
				q_mx_.unlock();
				return kInvalidID;
			}
			bool TryAccess(int i) {
				if (not IsValid(i) or nullptr == data_[i]) {
					return false;
				}
				mx_[i].lock();
				return true;
			}
			void EndAccess(int i) {
				mx_[i].unlock();
			}
			T& operator[](int index) noexcept {
				return *data_[index];
			}
			T& Get(int index) noexcept {
				return *data_[index];
			}
			void ReserveDelete(int i) {
				bool r{};
				mx_[i].lock();
				if (data_[i] != nullptr) {
					delete data_[i];
					data_[i] = nullptr;
					r = true;
				}
				mx_[i].unlock();

				if (r) {
					q_mx_.lock();
					queue_.push(i);
					q_mx_.unlock();
				}
			}

			static constexpr int kInvalidID{ -1 };
		private:
			bool IsValid(int i) const {
				if (0 <= i and i < kMaxElement) {
					return true;
				}
				return false;
			}
			const int kMaxElement;
			T** data_;
			std::mutex* mx_;
			std::queue<int> queue_;
			std::mutex q_mx_;
		};

		class Set {
		public:
			Set() = default;
			Set(int) {}
			bool Contains(int v) {
				return 0 != set_.count(v);
			}
			bool Insert(int v) {
				bool r{};
				mx_.lock();
				if (0 == set_.count(v)) {
					set_.insert(v);
					r = true;
				}
				else {
					r = false;
				}
				mx_.unlock();
				return r;
			}
			bool Remove(int v) {
				bool r{};
				mx_.lock();
				if (0 != set_.count(v)) {
					set_.erase(v);
					r = true;
				}
				else {
					r = false;
				}
				mx_.unlock();
				return r;
			}
			void GetElements(std::vector<int>& con) {
				mx_.lock();
				for (int i : set_) {
					con.push_back(i);
				}
				mx_.unlock();
			}
		private:
			std::mutex mx_;
			std::unordered_set<int> set_;
		};
	}
	

#if USES_NONBLOCKING_DS

	template<class T, double kTimeTolerance>
	using Queue = lf::RelaxedQueue<T, kTimeTolerance>;

	using Set = lf::Base15Tree;

	template<class T>
	using Array = lf::Array<T>;
#else
	template<class T, double kUnused>
	using Queue = blocking::Queue<T, kUnused>;

	using Set = blocking::Set;

	template<class T>
	using Array = blocking::Array<T>;
#endif
}