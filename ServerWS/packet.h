#pragma once
#include <array>
#include <utility>
#include <atomic>
#include <memory>

namespace packet {

	enum class Type : unsigned char {
		kPosition,
	};

	struct Base {
		Type t;
	};

	struct Position : Base {
		float x;
		float y;
		float z;
		Position(float x, float y, float z) 
			: Base{ Type::kPosition }, x{ x }, y{ y }, z{ z } {}
	};
}