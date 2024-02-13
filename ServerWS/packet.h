#pragma once
#include <print>
#include <tuple>
#include <memory>

namespace packet {
	enum class Type : unsigned char {
		kTest,
		kNewEntity,
		kPosition,
	};

	struct Base {
		Type type;
		unsigned char size;
	};

	struct Test : Base {
		int a, b, c;

		// 디버그 전용 디폴트 생성자
		Test() : Base{ Type::kTest, (unsigned char)sizeof(*this) - 4 },
			a{ 7 }, b{ 8 }, c{ 9 } {}

		Test(int a, int b, int c)
			: Base{ Type::kTest, (unsigned char)sizeof(*this) - 4 },
			a{ a }, b{ b }, c{ c } {}

		Test(char* p)
			: Base{ Type::kTest, (unsigned char)sizeof(*this) - 4 } {
			a = *(decltype(a)*)p;
			p += sizeof(a);
			b = *(decltype(b)*)p;
			p += sizeof(b);
			c = *(decltype(c)*)p;
		}
	};

	struct Position : Base {
		unsigned int id;
		float x;
		float y;
		float z;
		Position(unsigned int id, float x, float y, float z) 
			: Base{ Type::kPosition, (unsigned char)sizeof(*this) - 4},
				id{ id }, x{ x }, y{ y }, z{ z } {}

		Position(char* p)
			: Base{ Type::kPosition, (unsigned char)sizeof(*this) - 4 } {
			id = *(decltype(id)*)p;
			p += sizeof(id);
			x = *(decltype(x)*)p;
			p += sizeof(x);
			y = *(decltype(y)*)p;
			p += sizeof(y);
			z = *(decltype(z)*)p;
		}
	};

	struct NewEntity : Position {
		NewEntity(unsigned int id, float x, float y, float z)
			: Position{ id, x, y, z } {
			type = Type::kNewEntity;
		}
		NewEntity(char* p) : Position(p) {
			type = Type::kNewEntity;
		}
	};

	static std::shared_ptr<Base> Deserialize(char*& bytes)
	{
		Type type = (Type)((bytes++)[0]);

		switch (type) {
		case Type::kTest: {
			return std::make_shared<Test>(bytes);
		}
		case Type::kNewEntity: {
			return std::make_shared<NewEntity>(bytes);
		}
		case Type::kPosition: {
			return std::make_shared<Position>(bytes);
		}
		default: {
			std::print("Unknown Packet");
			return std::make_shared<Test>();
		}
		}
	}
}