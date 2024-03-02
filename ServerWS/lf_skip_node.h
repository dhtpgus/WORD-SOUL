#pragma once
#include <atomic>

namespace lf {
	template<class K, class V>
	struct SkipNode {
		using Epoch = unsigned long long;

		SkipNode() : k{}, v{}, top_level{}, next{}, retire_epoch{}, connections{ 1 } {}
		SkipNode(const K& k, V* v, int top) : k{ k }, v{ v }, top_level{ top },
			next{}, retire_epoch{}, connections{ top + 1 } {}
		~SkipNode() = default;
		SkipNode(const SkipNode&) = delete;
		SkipNode(SkipNode&&) = delete;
		SkipNode& operator=(const SkipNode&) = delete;
		SkipNode& operator=(SkipNode&&) = delete;

		void Set(int level, SkipNode* ptr, bool marking) {
			long long temp = reinterpret_cast<long long>(ptr);
			if (marking) temp |= 1;
			next[level] = reinterpret_cast<SkipNode*>(temp);
		}

		SkipNode* Get(int level, bool* removed) {
			//std::print("ddd {}\n", level);
			long long temp = reinterpret_cast<long long>(next[level]);
			*removed = temp & 1;
			return reinterpret_cast<SkipNode*>(temp & kAddrMask);
		}

		SkipNode* Get(int level) {
			long long temp = reinterpret_cast<long long>(next[level]);
			return reinterpret_cast<SkipNode*>(temp & kAddrMask);
		}

		bool CAS(int level, SkipNode* old_ptr, SkipNode* new_ptr,
			bool old_removed, bool new_removed) {

			long long old_v = reinterpret_cast<long long>(old_ptr);
			if (old_removed) old_v |= 1;
			long long new_v = reinterpret_cast<long long>(new_ptr);
			if (new_removed) new_v |= 1;
			return std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_llong*>(&next[level]),
				&old_v, new_v);
		}

		static constexpr int kMaxLevel = 10;
		static constexpr long long kAddrMask = 0xFFFF'FFFF'FFFF'FFFE;

		const K k;
		const std::shared_ptr<V> v;
		int top_level;
		SkipNode* volatile next[kMaxLevel + 1];
		Epoch retire_epoch;
		std::atomic_int connections;
	};
}