#pragma once
namespace lf {

	struct Node {
		using Level = unsigned long long;
		using Epoch = unsigned long long;
		Node* volatile next;
		void* v;
		Level level;
		Epoch retire_epoch;
		Node(void* p, Level level)
			: v{ p }, level{ level }, next{ nullptr }, retire_epoch{} {}
	};
}