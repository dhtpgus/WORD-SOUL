//---------------------------------------------------
// 
// packet.h - 패킷 관련 구조체 및 열거자 정의
// 
//---------------------------------------------------

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
	using Size = unsigned char;
	using Byte = unsigned char;

#pragma pack(push, 1)

	struct Base {
		Size size;
		Type type;
	};

	template<class Packet>
	static constexpr Size GetPacketSize()
	{
		return sizeof(Packet) - sizeof(Base);
	}

	template<class Packet>
	void Deserialize(Packet* packet, const Byte* byte)
	{
		memcpy(((Byte*)packet) + sizeof(Base), byte, GetPacketSize<Packet>());
	}

	struct Test : Base {
		Test() : Base{ GetPacketSize<decltype(*this)>(), Type::kTest },
			a{ 7 }, b{ 8 }, c{ 9 } {
		}

		Test(int a, int b, int c)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kTest }, a{ a }, b{ b }, c{ c } {}

		Test(Byte*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kTest } {
			Deserialize(this, byte);
		}

		int a, b, c;
	};

	struct Position : Base {
		Position() : Base{ GetPacketSize<decltype(*this)>(), Type::kPosition },
			id{}, x{}, y{}, z{} {}

		Position(int id, float x, float y, float z) 
			: Base{ GetPacketSize<decltype(*this)>(), Type::kPosition },
				id{ id }, x{ x }, y{ y }, z{ z } {}

		Position(Byte*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kPosition } {
			Deserialize(this, byte);
		}

		int id;
		float x;
		float y;
		float z;
	};

	struct NewEntity : Position {
		NewEntity(int id, float x, float y, float z, entity::Type et)
			: Position{ id, x, y, z }, entity_type{ et } {
			type = Type::kNewEntity;
			size = GetPacketSize<decltype(*this)>();
		}
		NewEntity(Byte*& byte)  {
			Deserialize(this, byte);
			type = Type::kNewEntity;
			size = GetPacketSize<decltype(*this)>();
		}

		entity::Type entity_type;
	};

#pragma pack(pop)

	static std::shared_ptr<Base> Deserialize(Byte*& bytes)
	{
		Size size = *(Size*)(bytes++);
		Type type = *(Type*)(bytes++);

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