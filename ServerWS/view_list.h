#pragma once
#include <vector>
#include "session.h"
#include "concurrent_ds.h"

using ViewList = concurrent::Set;

inline std::vector<ViewList*> view_lists(client::GetMaxClients());

inline void InitViewLists()
{
	for (auto& vl : view_lists) {
		vl = new ViewList{ 1 };
	}
}

inline void DeleteViewLists()
{
	for (auto vl : view_lists) {
		delete vl;
	}
}