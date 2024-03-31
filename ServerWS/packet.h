//---------------------------------------------------
// 
// packet.h - 패킷 관련 구조체 및 열거자 정의
// 
//---------------------------------------------------

#pragma once
#include <print>
#include <memory>
#include "entity.h"
#include "debug.h"

namespace packet {
	enum class Type : unsigned char {
		kTest,
		kNewEntity,
		kPosition = 129,
		kEnterParty,
		kRemoveEntity,
		kResult
	};
	using Size = unsigned char;

#pragma pack(push, 1)

	struct Base {
		Base(Size size, Type type) : size{ size }, type{ type } {}
		Size size{};
		Type type{};
	};

	template<class Packet>
	static constexpr Size GetPacketSize()
	{
		return sizeof(Packet) - sizeof(Base);
	}

	template<class Packet>
	void Deserialize(Packet* packet, char*& byte)
	{
		memcpy(((char*)packet) + sizeof(Base), byte, GetPacketSize<Packet>());
		byte += GetPacketSize<Packet>();
	}

	struct Test : Base {
		Test() : Base{ GetPacketSize<decltype(*this)>(), Type::kTest },
			a{ 7 }, b{ 8 }, c{ 9 } {
		}

		Test(int a, int b, int c)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kTest }, a{ a }, b{ b }, c{ c } {}

		Test(char*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kTest }, a{}, b{}, c{} {
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

		Position(char*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kPosition }, id{}, x{}, y{}, z{} {
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

		entity::Type entity_type;
	};

	struct EnterParty : Base {
		EnterParty(int id)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kEnterParty }, id{ id } {}
		EnterParty(char*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kEnterParty }, id{} {
			Deserialize(this, byte);
		}
		int id;
	};

	struct Result : Base {
		Result(bool value)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kResult }, value{ value } {}
		bool value;
	};

#pragma pack(pop)

	static void Free(char* p)
	{
		switch (static_cast<Type>(*p))
		{
		case Type::kTest:
			delete reinterpret_cast<packet::Test*>(p);
			break;
		case Type::kPosition:
			delete reinterpret_cast<packet::Position*>(p);
			break;
		case Type::kNewEntity:
			delete reinterpret_cast<packet::NewEntity*>(p);
			break;
		default:
			break;
		}
	}

	static std::string CheckBytes(char* bytes, int size)
	{
		std::string data;
		for (int i = 0; i < size; ++i) {
			data += std::format("{:02X} ", (unsigned int)(*(unsigned char*)(bytes + i)));
		}
		return std::format("send {} bytes: {}", size, data);
	}

	static std::string CheckBytes(const char* bytes, int size)
	{
		return CheckBytes((char*)bytes, size);
	}
}