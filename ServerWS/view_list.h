#pragma once
#include <vector>
#include "session.h"
#include "lf_base15_tree.h"

using ViewList = lf::Base15Tree;

inline std::vector<ViewList*> view_lists(client::GetMaxClients());

inline void InitViewLists()
{
	for (auto vl : view_lists) {
		vl = new ViewList{ 1 };
	}
}
