//---------------------------------------------------
// 
// lf_node.h - lf::Node 구조체 정의
// 
//---------------------------------------------------

#pragma once

namespace lf {
	struct Node {
		using Level = unsigned long long;
		using Epoch = unsigned long long;
		using Value = void*;
		Node* volatile next;
		Value v;
		Level level;
		Epoch retire_epoch;

		Node(Value v, Level level) noexcept
			: v{ v }, level{ level }, next{ nullptr }, retire_epoch{} {}
		void Reset(Value rs_v, Level rs_level) {
			v = rs_v;
			level = rs_level;
			next = nullptr;
			retire_epoch = 0;
		}
	};
}