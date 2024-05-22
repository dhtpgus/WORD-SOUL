#include "lf_base15_tree.h"

namespace lf {
	void Base15Tree::GetElements(std::forward_list<int>& fl)
	{
		if (not IsLeaf()) {
			for (int i = 0; i < kChunkSize; ++i) {
				auto child = children_[i].TryAccess();
				if (nullptr != child) {
					child->GetElements(fl);
					children_[i].EndAccess();
				}
			}
			return;
		}
		auto q = GetQWord(0);
		while (true) {
			if ((q & 0xF) == 0) {
				return;
			}
			fl.push_front(children_[(q & 0xF) - 1].v);
			q /= 16;
		}
	}

	void Base15Tree::GetElements(std::vector<int>& vec)
	{
		if (not IsLeaf()) {
			for (int i = 0; i < kChunkSize; ++i) {
				auto child = children_[i].TryAccess();
				if (nullptr != child) {
					child->GetElements(vec);
					children_[i].EndAccess();
				}
			}
			return;
		}
		auto q = GetQWord(0);
		while (true) {
			if ((q & 0xF) == 0) {
				return;
			}
			vec.push_back(children_[(q & 0xF) - 1].v);
			q /= 16;
		}
	}

	void Base15Tree::GetElements(std::unordered_set<int>& uset)
	{
		if (not IsLeaf()) {
			for (int i = 0; i < kChunkSize; ++i) {
				auto child = children_[i].TryAccess();
				if (nullptr != child) {
					child->GetElements(uset);
					children_[i].EndAccess();
				}
			}
			return;
		}
		auto q = GetQWord(0);
		while (true) {
			if ((q & 0xF) == 0) {
				return;
			}
			uset.insert(children_[(q & 0xF) - 1].v);
			q /= 16;
		}
	}
}