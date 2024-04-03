#pragma once
#include <vector>
#include <atomic>
#include "thread.h"

template<class T>
class FreeList {
public:
	FreeList() = delete;
	FreeList(int el_num) : pointers_{} {
		pointers_.reserve(el_num);
	}
	~FreeList() {
		for (auto p : pointers_) {
			delete p;
		}
	}

	auto Get() noexcept {
		if (pointers_.empty()) {
			return new T{};
		}
		auto p =  pointers_.back();
		pointers_.pop_back();
		p->Reset();
		return p;
	}

	template<class... Value>
	auto Get(Value... values) noexcept {
		if (pointers_.empty()) {
			return new T{ values... };
		}
		auto p = pointers_.back();
		pointers_.pop_back();
		p->Reset(values...);
		return p;
	}
	void Collect(T* ptr) noexcept {
		if (pointers_.size() == pointers_.capacity()) {
			delete ptr;
			return;
		}
		pointers_.push_back(ptr);
	}
private:
	std::vector<T*> pointers_;
};

template <class T>
inline thread_local FreeList<T> free_list{ 100 };