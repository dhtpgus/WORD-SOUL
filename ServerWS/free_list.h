#pragma once
#include <vector>
#include <print>
#include "thread.h"
#include "over_ex.h"

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
	template<class Type>
	auto Get() noexcept {
		if (pointers_.empty()) {
			return new Type{};
		}
		auto p = reinterpret_cast<Type*>(pointers_.back());
		pointers_.pop_back();
		p->Reset();
		return p;
	}

	template<class Type, class... Value>
	auto Get(Value... values) noexcept {
		if (pointers_.empty()) {
			return new Type{ values... };
		}
		auto p = reinterpret_cast<Type*>(pointers_.back());
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

namespace free_list {
	inline thread_local FreeList<OverEx> ox{ 100 };
}