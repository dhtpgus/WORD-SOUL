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
		Node(Value v, Level level)
			: v{ v }, level{ level }, next{ nullptr }, retire_epoch{} {}
	};
}