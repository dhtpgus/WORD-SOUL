#pragma once
#include "lf_skip_node.h"
#include "ebr.h"
#include "random_number_generator.h"

namespace lf {

	template<class K, class V>
	class SkipList {
	public:
		SkipList() = delete;
		SkipList(int num_thread) /* : ebr_{num_thread}*/ {
			for (int i = 0; i <= SkipNode<K, V>::kMaxLevel; ++i) {
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

		bool Add(const K& key, V* value) {

			int level = 0;
			for (level = 0; level < SkipNode<K, V>::kMaxLevel; ++level) {
				if (rng.Rand(0, 1) == 1) {
					break;
				}
			}
			
			SkipNode<K, V>* node{ new SkipNode<K, V>{ key, value, level } };

			SkipNode<K, V>* prev[SkipNode<K, V>::kMaxLevel + 1];
			SkipNode<K, V>* curr[SkipNode<K, V>::kMaxLevel + 1];

			while (true) {
				if (true == Find(key, prev, curr)) {
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
						Find(key, prev, curr);
					}
				}
				return true;
			}
		}

		bool Remove(const K& key) {
			SkipNode<K, V>* prev[SkipNode<K, V>::kMaxLevel + 1];
			SkipNode<K, V>* curr[SkipNode<K, V>::kMaxLevel + 1];
			SkipNode<K, V>* victim = nullptr;
			if (false == Find(key, prev, curr)) {
				return false;
			}
			auto* r_node = curr[0];
			int top_level = curr[0]->top_level;
			for (int i = top_level; i >= 1; --i) { // 0 레벨 빼고 나머지 링크 제거
				while (true) {
					bool removed = false;
					auto* succ = r_node->Get(i, &removed);
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
				auto succ = r_node->Get(0, &removed);
				if (true == removed) {
					return false;
				}
				if (true == r_node->CAS(0, succ, succ, false, true)) {
					Find(key, prev, curr);
					return true;
				}
			}
		}

		bool Contains(const K& key) {
			SkipNode<K, V>* prev = &head_;
			SkipNode<K, V>* curr{};
			SkipNode<K, V>* succ{};
			bool removed = false;

			for (int i = SkipNode<K, V>::kMaxLevel; i >= 0; --i) {
				curr = prev->Get(i);
				while (true) {
					succ = curr->Get(i, &removed);
					while (removed) {
						curr = curr->Get(i);
						succ = curr->Get(i, &removed);
					}
					if ((curr->k <=> key) < 0) {
						prev = curr;
						curr = succ;
					}
					else {
						break;
					}
				}
			}
			return (key <=> curr->k) == 0;
		}

		void Clear() {
			auto* p = head_.Get(0);
			while (p != &tail_) {
				auto* t = p;
				p = p->Get(0);
				delete t;
			}
			for (int i = 0; i <= SkipNode<K, V>::kMaxLevel; ++i) {
				head_.Set(i, &tail_, false);
			}
		}

	private:
		bool Find(const K& k, SkipNode<K, V>* prev[], SkipNode<K, V>* curr[]) {
		retry:
			prev[SkipNode<K, V>::kMaxLevel] = &head_;
			for (int cl = SkipNode<K, V>::kMaxLevel; cl >= 0; --cl) {
				if (cl != SkipNode<K, V>::kMaxLevel)
					prev[cl] = prev[cl + 1];

				while (true) {
					curr[cl] = prev[cl]->Get(cl);
					bool removed = false;

					auto* succ = curr[cl]->Get(cl, &removed);
					while (true == removed) {
						if (false == prev[cl]->CAS(cl, curr[cl], succ, false, false)) {
							goto retry;
						}
						curr[cl] = succ;
						succ = curr[cl]->Get(cl, &removed);
					}

					if (curr[cl] == &tail_) {
						break;
					}

					if ((curr[cl]->k <=> k) >= 0) {
						break;
					}
					prev[cl] = curr[cl];
				}
			}

			if (curr[0] == &head_ or curr[0] == &tail_) {
				return false;
			}

			return ((curr[0]->k) <=> k) == 0;
		}

		SkipNode<K, V> head_, tail_;
		//EBR ebr_;
	};
}