#include "lf_child_node.h"
#include "lf_base15_tree.h"

namespace lf {
	thread_local Base15Tree* freed_ptr[8]{};

	ChildNode::~ChildNode()
	{
		if (ref_cnt > 0) {
			delete reinterpret_cast<Base15Tree*>(GetData());
		}
	}

	Base15Tree* ChildNode::StartAccess(int layer)
	{
		while (true) {
			size_t local_cnt = ref_cnt;
			auto local_ref_cnt = GetRefCnt(local_cnt);
			if (local_ref_cnt <= 0) {
				auto new_ptr = nullptr == freed_ptr[layer] ? new Base15Tree{ layer } : freed_ptr[layer];
				if (true == CASDataInit(new_ptr)) {
					freed_ptr[layer] = nullptr;
					return new_ptr;
				}
				freed_ptr[layer] = new_ptr;
				continue;
			}
			if (ref_cnt.compare_exchange_strong(local_cnt, local_cnt + kRefCntDiff)) {
				return GetData(local_cnt);
			}
		}
	}

	Base15Tree* ChildNode::TryAccess()
	{
		while (true) {
			size_t local_cnt = ref_cnt;
			auto local_ref_cnt = GetRefCnt(local_cnt);
			if (local_ref_cnt <= 0) {
				return nullptr;
			}
			if (ref_cnt.compare_exchange_strong(local_cnt, local_cnt + kRefCntDiff)) {
				return GetData(local_cnt);
			}
		}
	}

	void ChildNode::EndAccess()
	{
		ref_cnt -= kRefCntDiff;
		TryDelete();
	}

	void ChildNode::TryDelete()
	{
		void* old_ptr{};
		size_t local_cnt = ref_cnt;
		auto local_ref_cnt = GetRefCnt(local_cnt);
		if (local_ref_cnt == 1) {
			if (CASDataEmpty(local_cnt, old_ptr)) {
				delete reinterpret_cast<ChildNode*>(old_ptr);
			}
		}
	}
}