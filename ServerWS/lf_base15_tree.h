#include <vector>
#include <forward_list>
#include <unordered_set>
#include <stack>
#include <print>
#include <atomic>
#include "lf_child_node.h"

namespace lf {
	enum class State : char {
		kEmpty, kInserting, kInserted, kRemoving,
	};

	struct alignas(16) Bit128 {
		volatile unsigned char bytes[16];
	};

	class Base15Tree {
	public:
		using RefCntVector = std::vector<std::atomic_ullong*>;

		Base15Tree() {
			for (auto& i : state) {
				i = State::kEmpty;
			}
			memset(const_cast<unsigned char*>(data.bytes), 0, 16);
			data.bytes[15] = 1;
		}

		// 15의 (layer + 1)승 개만큼의 원소 보유 가능
		Base15Tree(int layer) {
			if (layer > 4) {
				std::print("[Error] The layer value must be at most 4.\n");
			}
			for (auto& i : state) {
				i = State::kEmpty;
			}
			memset(const_cast<unsigned char*>(data.bytes), 0, 16);
			data.bytes[15] = layer;
		}

		bool Insert(int v) {
			bool result{};
			RefCntVector ref_cnts;
			ref_cnts.reserve(GetLayer());
			Insert(v, result, v, ref_cnts);
			return result;
		}

		bool Remove(int v) {
			bool result{};
			RefCntVector ref_cnts;
			ref_cnts.reserve(GetLayer());
			Remove(v, result, ref_cnts);
			return result;
		}

		bool Contains(int v) {
			if (not IsLeaf()) {
				auto child = children[v % kChunkSize].TryAccess();
				if (nullptr == child) {
					return false;
				}

				auto r = child->Contains(v / kChunkSize);
				children[v % kChunkSize].EndAccess();
				return r;
			}

			State local_value = static_cast<State>(state[v].load());

			return local_value == State::kInserted;
		}

		void GetElements(std::forward_list<int>&);
		void GetElements(std::vector<int>&);
		void GetElements(std::unordered_set<int>&);

	private:
		size_t GetQWord(int diff) const {
			return *reinterpret_cast<const volatile size_t*>(data.bytes + diff);
		}

		bool IsLeaf() const {
			return 0 == GetLayer();
		}

		char GetLayer() const {
			return data.bytes[15];
		}

		bool CASChunk(int diff, size_t expected, size_t desired) {
			return std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_ullong*>(data.bytes + diff),
				&expected, desired);
		}

		void Insert(int v, bool& result, int original_v, RefCntVector& ref_cnts) {
			if (not IsLeaf()) {
				auto child = children[v % kChunkSize].StartAccess(GetLayer() - 1);
				auto child_ref_cnt = &children[v % kChunkSize].ref_cnt;
				ref_cnts.push_back(child_ref_cnt);

				child->Insert(v / kChunkSize, result, original_v, ref_cnts);
				children[v % kChunkSize].EndAccess();
				return;
			}

			auto expected_state = State::kEmpty;
			auto desired_state = State::kInserting;
			if (false == state[v].compare_exchange_strong(expected_state, desired_state)) {
				return;
			}
			while (true) {
				auto q = GetQWord(0);
				int byte{};
				auto desired = q * 16 + (1ULL + v);

				if (false == CASChunk(0, q, desired)) {
					continue; // retry
				}

				for (auto ref_cnt : ref_cnts) {
					ref_cnt->fetch_add(ChildNode::kRefCntDiff);
				}

				state[v] = State::kInserted;

				result = true;

				children[v].v = original_v;

				return;
			}
		}

		void Remove(int v, bool& result, RefCntVector& ref_cnts) {
			if (not IsLeaf()) {
				auto child = children[v % kChunkSize].TryAccess();
				auto child_ref_cnt = &children[v % kChunkSize].ref_cnt;
				ref_cnts.push_back(child_ref_cnt);
				if (nullptr == child) {
					return;
				}
				child->Remove(v / kChunkSize, result, ref_cnts);
				children[v % kChunkSize].EndAccess();
				return;
			}

			auto expected_state = State::kInserted;
			auto desired_state = State::kRemoving;
			if (false == state[v].compare_exchange_strong(expected_state, desired_state)) {
				return;
			}
			while (true) {
				size_t local_bytes = GetQWord(0);

				int byte{ 0 };
				size_t find = 1ULL + v;
				for (; byte < 15; ++byte) {
					if ((local_bytes & 0xF) == find) {
						break;
					}
					local_bytes /= 16;
				}

				auto qword = GetQWord(byte / 2);
				if (byte % 2 == 0) {
					if ((qword & 0xF) != find or false == CASChunk(byte / 2, qword, qword / 16)) {
						continue; // retry
					}
				}
				else {
					size_t desired = qword;
					auto c = desired & 0xF;
					desired /= 16;
					desired &= 0xFFFF'FFFF'FFFF'FFF0;
					desired |= c;
					if ((qword & 0xF0) != find * 16 or false == CASChunk(byte / 2, qword, desired)) {
						continue; // retry
					}
				}

				for (auto ref_cnt : ref_cnts) {
					ref_cnt->fetch_sub(ChildNode::kRefCntDiff);
				}

				state[v] = State::kEmpty;

				result = true;

				return;
			}
		}

		static constexpr auto kChunkSize{ 15 };
		Bit128 data;
		std::atomic<State> state[kChunkSize];
		ChildNode children[kChunkSize];
	};
}