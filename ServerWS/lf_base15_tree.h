#include <unordered_set>
#include <set>
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

	struct Base15Tree {
		using Set = std::unordered_set<int>;
		//using Set = std::set<int>;
		using RefCntVector = std::vector<std::atomic_ullong*>;

		// 8의 (layer + 1)승 개만큼의 원소 보유 가능
		Base15Tree(int layer) {
			for (auto& i : state) {
				i = State::kEmpty;
			}
			memset(const_cast<unsigned char*>(data.bytes), 0, 16);
			data.bytes[15] = layer;
		}

		size_t GetQWord(int diff) const {
			return *reinterpret_cast<const volatile size_t*>(data.bytes + diff);
		}

		bool CASChunk(int diff, size_t expected, size_t desired) {
			return std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_ullong*>(data.bytes + diff),
				&expected, desired);
		}

		bool Insert(int v) {
			bool result{};
			RefCntVector ref_cnts;
			ref_cnts.reserve(GetLayer());
			Insert(v, result, v, ref_cnts);
			return result;
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
		retry:
			auto q = GetQWord(0);
			int byte{};

			auto desired = q * 16 + (1ULL + v);

			if (false == CASChunk(0, q, desired)) {
				goto retry;
			}

			for (auto ref_cnt : ref_cnts) {
				ref_cnt->fetch_add(ChildNode::kRefCntDiff);
			}

			state[v] = State::kInserted;

			result = true;

			children[v].v = original_v;

			return;
		}

		bool Remove(int v) {
			bool result{};
			RefCntVector ref_cnts;
			ref_cnts.reserve(GetLayer());
			Remove(v, result, ref_cnts);
			return result;
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
		retry:
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
					goto retry;
				}
			}
			else {
				size_t desired = qword;
				auto c = desired & 0xF;
				desired /= 16;
				desired &= 0xFFFF'FFFF'FFFF'FFF0;
				desired |= c;
				if ((qword & 0xF0) != find * 16 or false == CASChunk(byte / 2, qword, desired)) {
					goto retry;
				}
			}

			for (auto ref_cnt : ref_cnts) {
				ref_cnt->fetch_sub(ChildNode::kRefCntDiff);
			}

			state[v] = State::kEmpty;

			result = true;

			return;
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

		bool IsLeaf() const {
			return 0 == GetLayer();
		}

		int Count() const {
			auto q = GetQWord(0);
			int cnt{};

			//std::print("{:x}\n", q);

			if (not IsLeaf()) {
				while (true) {
					if ((q & 0xF) == 0) {
						break;
					}
					auto child = reinterpret_cast<Base15Tree*>(children[(q & 0xF) - 1].GetData());
					cnt += child->Count();
					q /= 16;
				}
				return cnt;
			}
			return GetSize(q);
		}

		int GetSize(size_t q) const {
			int cnt{ 0 };
			while (true) {
				if ((q & 0xF) == 0) {
					break;
				}
				q /= 16;
				cnt += 1;
			}
			//std::print("{} ", cnt);
			return cnt;
		};

		char GetLayer() const {
			return data.bytes[15];
		}

		void GetElements2(Set& s) {
			auto q = GetQWord(0);

			if (not IsLeaf()) {
				while (true) {
					if ((q & 0xF) == 0) {
						break;
					}
					auto child = children[(q & 0xF) - 1].TryAccess();
					if (nullptr != child) {
						child->GetElements2(s);
						children[(q & 0xF) - 1].EndAccess();
					}
					q /= 16;
				}
				return;
			}
			while (true) {
				if ((q & 0xF) == 0) {
					break;
				}
				s.insert(children[(q & 0xF) - 1].v);
				q /= 16;
			}
			return;
		}

		void GetElements(Set& s) {
			if (not IsLeaf()) {
				for (int i = 0; i < kChunkSize; ++i) {
					auto child = children[i].TryAccess();
					if (nullptr != child) {
						child->GetElements(s);
						children[i].EndAccess();
					}
				}
				return;
			}
			auto q = GetQWord(0);
			while (true) {
				if ((q & 0xF) == 0) {
					return;
				}
				s.insert(children[(q & 0xF) - 1].v);
				q /= 16;
			}
		}


		static constexpr auto kChunkSize{ 15 };
		Bit128 data;
		std::atomic<State> state[kChunkSize];
		ChildNode children[kChunkSize];
	};
}