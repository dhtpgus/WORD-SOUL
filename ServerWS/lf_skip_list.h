#pragma once
#include "lf_skip_node.h"
#include "ebr.h"
#include "random_number_generator.h"

namespace lf {

	template<class T>
	class SkipList {
	public:
		SkipList() = delete;
		SkipList(int num_thread) : ebr_{ num_thread } {
			head_.v = reinterpret_cast<SkipNode::Value>(0);
			tail_.v = reinterpret_cast<SkipNode::Value>(0xFFFF'FFFF'FFFF'FFFF);
			for (int i = 0; i <= SkipNode::kMaxLevel; ++i) {
				head_.Set(i, &tail_, false);
			}
		}
		~SkipList() {
			Clear();
		}
		SkipList(const SkipList&) = delete;
		SkipList(SkipList&&) = delete;
		SkipList& operator=(const SkipList&) = delete;
		SkipList& operator=(SkipList&&) = delete;

		bool Add(T* ptr) {
			SkipNode::Value x{ ptr };

			int level = 0;
			for (level = 0; level < SkipNode::kMaxLevel; ++level) {
				if (rng.Rand(0, 1) == 1) {
					break;
				}
			}

			SkipNode* node{ new SkipNode{ x, level } };

			SkipNode* prev[SkipNode::kMaxLevel + 1];
			SkipNode* curr[SkipNode::kMaxLevel + 1];

			while (true) {
				if (true == Find(x, prev, curr)) {
					return false;
				}
				for (int i = 0; i <= level; ++i) {
					node->Set(i, curr[i], false);
				}

				node->Set(0, curr[0], false);
				if (false == prev[0]->CAS(0, curr[0], node, false, false)) {
					continue;
				}
				for (int i = 1; i <= level; ++i) {
					while (true) {
						if (prev[i]->CAS(i, curr[i], node, false, false)) {
							break;
						}
						Find(x, prev, curr);
					}
				}
				return true;
			}
		}

		bool Remove(T* ptr) {
			SkipNode::Value x{ ptr };

			SkipNode* prev[SkipNode::kMaxLevel + 1];
			SkipNode* curr[SkipNode::kMaxLevel + 1];
			SkipNode* victim = nullptr;
			if (false == Find(x, prev, curr)) {
				return false;
			}
			SkipNode* r_node = curr[0];
			int top_level = curr[0]->top_level;
			for (int i = top_level; i >= 1; --i) { // 0 레벨 빼고 나머지 링크 제거
				while (true) {
					bool removed = false;
					SkipNode* succ = r_node->Get(i, &removed);
					if (true == removed) {
						break;
					}
					bool ret = r_node->CAS(i, succ, succ, false, true);
					if (true == ret) {
						break;
					}
				}
			}
			while (true) {
				bool removed = false;
				SkipNode* succ = r_node->Get(0, &removed);
				if (true == removed) {
					return false;
				}
				if (true == r_node->CAS(0, succ, succ, false, true)) {
					Find(x, prev, curr);
					return true;
				}
			}
		}

		bool Contains(T& value) {
			SkipNode* prev = &head_;
			SkipNode* curr{};
			SkipNode* succ{};
			bool removed = false;

			for (int i = SkipNode::kMaxLevel; i >= 0; --i) {
				curr = prev->Get(i);
				while (true) {
					succ = curr->Get(i, &removed);
					while (removed) {
						curr = curr->Get(i);
						succ = curr->Get(i, &removed);
					}
					if ((*(T*)(curr->v) <=> value) < 0) {
						prev = curr;
						curr = succ;
					}
					else {
						break;
					}
				}
			}
			return (value <=> *(T*)(curr->v)) == 0;
		}

		void Clear() {
			SkipNode* p = head_.Get(0);
			while (p != &tail_) {
				SkipNode* t = p;
				p = p->Get(0);
				delete t;
			}
			for (int i = 0; i <= SkipNode::kMaxLevel; ++i) {
				head_.Set(i, &tail_, false);
			}
		}

	private:
		bool Find(SkipNode::Value x, SkipNode* prev[], SkipNode* curr[]) {
		retry:
			prev[SkipNode::kMaxLevel] = &head_;
			for (int cl = SkipNode::kMaxLevel; cl >= 0; --cl) {
				if (cl != SkipNode::kMaxLevel)
					prev[cl] = prev[cl + 1];

				while (true) {
					curr[cl] = prev[cl]->Get(cl);
					bool removed = false;

					SkipNode* succ = curr[cl]->Get(cl, &removed);
					while (true == removed) {
						if (false == prev[cl]->CAS(cl, curr[cl], succ, false, false)) {
							goto retry;
						}
						curr[cl] = succ;
						succ = curr[cl]->Get(cl, &removed);
					}

					if (curr[cl]->v == tail_.v) {
						break;
					}

					if ((*(T*)(curr[cl]->v) <=> *(T*)x) >= 0) {
						break;
					}
					prev[cl] = curr[cl];
				}
			}

			if (curr[0]->v == head_.v or curr[0]->v == tail_.v) {
				return false;
			}

			return (*(T*)(curr[0]->v) <=> *(T*)x) == 0;
		}

		SkipNode head_, tail_;
		EBR ebr_;
	};
}