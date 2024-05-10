#pragma once
#include <atomic>

namespace lf {
	class Base15Tree;

	struct ChildNode {
		ChildNode() = default;
		~ChildNode();

		auto GetRefCnt(size_t v) const {
			return static_cast<unsigned short>((v & 0xFFFF'0000'0000'0000) >> 48);
		}

		Base15Tree* GetData(size_t qword) const {
			return reinterpret_cast<Base15Tree*>(qword & 0x0000'FFFF'FFFF'FFFF);
		}

		Base15Tree* GetData(void* ptr) const {
			return reinterpret_cast<Base15Tree*>(reinterpret_cast<size_t>(ptr) & 0x0000'FFFF'FFFF'FFFF);
		}

		bool CASDataEmpty(size_t expected, void*& ptr) {
			auto desired = kDeleteValue;
			return ref_cnt.compare_exchange_strong(expected, desired);
		}
		bool CASDataInit(void* new_ptr) {
			size_t expected = kDeleteValue;
			size_t qword = reinterpret_cast<size_t>(new_ptr) | 0x0002'0000'0000'0000;
			return ref_cnt.compare_exchange_strong(expected, qword);
		}

		void EndAccess();
		//void ReserveDelete();

		void TryDelete();

		Base15Tree* StartAccess(int layer);
		Base15Tree* TryAccess();

		static constexpr size_t kDeleteValue{ 0xFFFF'0000'0000'0000 };
		static constexpr size_t kRefCntDiff{ 0x0001'0000'0000'0000 };

		std::atomic_ullong ref_cnt{ 0xFFFF'0000'0000'0000 };
		int v{ -1 };
	};
}