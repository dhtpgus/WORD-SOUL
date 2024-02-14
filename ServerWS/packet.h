#pragma once
#include <print>
#include <tuple>
#include <memory>
#include "entity.h"

namespace packet {
	enum class Type : unsigned char {
		kTest,
		kNewEntity,
		kPosition,
	};

	template<class T>
	static T Deserialize(char*& p) {
		T data = *(T*)p;
		p += sizeof(data);
		return data;
	}

	struct Base {
		Type type;
		unsigned char size;
	};

	struct Test : Base {
		// 디버그 전용 디폴트 생성자
		Test() : Base{ Type::kTest, (unsigned char)sizeof(*this) - 4 },
			a{ 7 }, b{ 8 }, c{ 9 } {}

		Test(int a, int b, int c)
			: Base{ Type::kTest, (unsigned char)sizeof(*this) - 4 },
			a{ a }, b{ b }, c{ c } {}

		Test(char*& p)
			: Base{ Type::kTest, (unsigned char)sizeof(*this) - 4 } {
			a = Deserialize<decltype(a)>(p);
			b = Deserialize<decltype(b)>(p);
			c = Deserialize<decltype(c)>(p);
		}

		int a, b, c;
	};

	struct Position : Base {
		Position(unsigned int id, float x, float y, float z) 
			: Base{ Type::kPosition, (unsigned char)sizeof(*this) - 4},
				id{ id }, x{ x }, y{ y }, z{ z } {}

		Position(char*& p)
			: Base{ Type::kPosition, (unsigned char)sizeof(*this) - 4 } {
			id = Deserialize<decltype(id)>(p);
			x = Deserialize<decltype(x)>(p);
			y = Deserialize<decltype(y)>(p);
			z = Deserialize<decltype(z)>(p);
		}

		unsigned int id;
		float x;
		float y;
		float z;
	};

	struct NewEntity : Position {
		NewEntity(unsigned int id, float x, float y, float z, entity::Type et)
			: Position{ id, x, y, z }, entity_type{ et } {
			type = Type::kNewEntity;
			entity_type = et;
		}
		NewEntity(char*& p) : Position{ p } {
			type = Type::kNewEntity;
			entity_type = Deserialize<decltype(entity_type)>(p);
		}

		entity::Type entity_type;
	};

	static std::shared_ptr<Base> Deserialize(char*& bytes)
	{
		Type type = *(Type*)((bytes++));

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
			std::print("Unknown Packet: {}\n", (int)type);
			return std::make_shared<Test>();
		}
		}
	}
}