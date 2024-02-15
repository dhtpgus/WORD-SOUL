#pragma once
#include "lf_skip_node.h"

namespace lf {
	class T;

	class SkipList {
		SkipNode head, tail;
	public:
		SkipList() {
			head.v = reinterpret_cast<SkipNode::Value>(0);
			tail.v = reinterpret_cast<SkipNode::Value>(0xFFFF'FFFF'FFFF'FFFF);
			for (int i = 0; i <= SkipNode::kMaxLevel; ++i) {
				head.Set(i, &tail, false);
			}
		}
		~SkipList() {
			clear();
		}

		bool Find(int x, SkipNode* prev[], SkipNode* curr[])
		{
		retry:
			prev[SkipNode::kMaxLevel] = &head;
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

					if (curr[cl]->v >= x) {
						break;
					}
					prev[cl] = curr[cl];
				}
			}
			return curr[0]->v == x;
		}

		bool ADD(int x)
		{
			int level = 0;
			for (level = 0; level < TOP_LEVEL; ++level)
				if (rand() % 2 == 1) break;

			LFSKNODE* node{ new LFSKNODE{ x, level } };

			LFSKNODE* prev[TOP_LEVEL + 1];
			LFSKNODE* curr[TOP_LEVEL + 1];

			while (true) {
				if (true == Find(x, prev, curr)) {
					return false;
				}
				for (int i = 0; i <= level; ++i) {
					node->set(i, curr[i], false);
				}

				node->set(0, curr[0], false);
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

		bool REMOVE(int x)
		{
			LFSKNODE* prev[TOP_LEVEL + 1];
			LFSKNODE* curr[TOP_LEVEL + 1];
			LFSKNODE* victim = nullptr;
			if (false == Find(x, prev, curr)) {
				return false;
			}
			LFSKNODE* r_node = curr[0];
			int top_level = curr[0]->top_level;
			for (int i = top_level; i >= 1; --i) { // 0 레벨 빼고 나머지 링크 제거
				while (true) {
					bool removed = false;
					LFSKNODE* succ = r_node->get(i, &removed);
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
				LFSKNODE* succ = r_node->get(0, &removed);
				if (true == removed) {
					return false;
				}
				if (true == r_node->CAS(0, succ, succ, false, true)) {
					Find(x, prev, curr);
					return true;
				}
			}
		}

		bool CONTAINS(int x)
		{
			LFSKNODE* prev = &head;
			LFSKNODE* curr{};
			LFSKNODE* succ{};
			bool removed = false;

			for (int i = TOP_LEVEL; i >= 0; --i) {
				curr = prev->get(i);
				while (true) {
					succ = curr->get(i, &removed);
					while (removed) {
						curr = curr->get(i);
						succ = curr->get(i, &removed);
					}
					if (curr->v < x) {
						prev = curr;
						curr = succ;
					}
					else {
						break;
					}
				}
			}
			return x == curr->v;
		}
		void print20()
		{
			LFSKNODE* p = head.get(0);
			for (int i = 0; i < 20; ++i) {
				if (p == &tail) break;
				cout << p->v << ", ";
				p = p->get(0);
			}
			cout << endl;
		}

		void clear()
		{
			LFSKNODE* p = head.get(0);
			while (p != &tail) {
				LFSKNODE* t = p;
				p = p->get(0);
				delete t;
			}
			for (int i = 0; i <= TOP_LEVEL; ++i) {
				head.set(i, &tail, false);
			}
		}
	};
}