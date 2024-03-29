#pragma once
#include <vector>

template<class T>
class FreeList {
public:
	FreeList() = delete;
	FreeList(int el_num)
		: pointers_{ new T*[el_num]{} }, current_el_num_{ -1 }, max_el_num_{ el_num } {}
	~FreeList() {
		for (int i = 0; i < max_el_num_; ++i) {
			delete pointers_[i];
		}
		delete[] pointers_;
	}
	template<class Type>
	auto Allocate() noexcept {
		if (-1 == current_el_num_) {
			return new Type{};
		}
		auto p = pointers_[current_el_num_--];
		return p;
	}
	template<class Type, class... Value>
	auto Allocate(Value... values) noexcept {
		if (-1 == current_el_num_) {
			return new Type{ values... };
		}
		auto p = pointers_[current_el_num_--];
		p->Reallocate(values...);
		return p;
	}
	void Collect(T* ptr) noexcept {
		if (current_el_num_ == max_el_num_) {
			delete ptr;
			return;
		}
		pointers_[current_el_num_++] = ptr;
	}
private:
	T** pointers_;
	int current_el_num_;
	int max_el_num_;
};