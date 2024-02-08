#pragma once
#include <unordered_map>
#include <utility>
#include <atomic>
#include <memory>

namespace packet {

	enum class Type : unsigned char {
		kTest,
		kPosition,
	};

	struct Base {
		Type type;
		unsigned char size;
	};

	struct Test : Base {
		int a, b, c;
		Test(int a, int b, int c)
			: Base{ Type::kPosition, (unsigned char)sizeof(*this) - 4 },
			a{ a }, b{ b }, c{ c } {}
	};

	struct Position : Base {
		float x;
		float y;
		float z;
		Position(float x, float y, float z) 
			: Base{ Type::kPosition, (unsigned char)sizeof(*this) - 4},
				x{ x }, y{ y }, z{ z } {}
	};
}