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
		kSCNewEntity,
		kSCPosition,
		kSCRemoveEntity,
		kSCResult,
		kSCCheckConnection,

		kCSJoinParty = 128,
		kCSPosition,
		kCSLeaveParty,
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

	struct SCPosition : Base {
		SCPosition() : Base{ GetPacketSize<decltype(*this)>(), Type::kSCPosition },
			id{}, x{}, y{}, z{} {}

		SCPosition(int id, float x, float y, float z) 
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCPosition },
			id{ id }, x{ x }, y{ y }, z{ z } {}

		int id;
		float x;
		float y;
		float z;
	};

	struct SCNewEntity : SCPosition {
		SCNewEntity(int id, float x, float y, float z, entity::Type et)
			: SCPosition{ id, x, y, z }, entity_type{ et } {
			type = Type::kSCNewEntity;
			size = GetPacketSize<decltype(*this)>();
		}

		entity::Type entity_type;
	};

	struct SCRemoveEntity : Base {
		SCRemoveEntity() : Base{ GetPacketSize<decltype(*this)>(), Type::kSCRemoveEntity },
			id{} {}

		SCRemoveEntity(int id)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCRemoveEntity }, id{ id } {}

		int id;
	};

	struct SCResult : Base {
		SCResult(bool value)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCResult }, value{ value } {}
		bool value;
	};

	struct SCCheckConnection : Base {
		SCCheckConnection()
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCCheckConnection }, value{ 0x55 } {}
		char value;
	};

	struct CSJoinParty : Base {
		CSJoinParty(int id)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSJoinParty }, id(id) {}
		CSJoinParty(char*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSJoinParty }, id{} {
			Deserialize(this, byte);
		}
		unsigned short id;
	};

	struct CSLeaveParty : Base {
		CSLeaveParty(int id)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSLeaveParty }, id(id) {}
		CSLeaveParty(char*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSLeaveParty }, id{} {
			Deserialize(this, byte);
		}
		unsigned short id;
	};

	struct CSPosition : Base {
		CSPosition() : Base{ GetPacketSize<decltype(*this)>(), Type::kCSPosition },
			x{}, y{}, z{} {}

		CSPosition(float x, float y, float z)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSPosition }, x{ x }, y{ y }, z{ z } {}

		CSPosition(char*& byte)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSPosition }, x{}, y{}, z{} {
			Deserialize(this, byte);
		}

		float x;
		float y;
		float z;
	};

#pragma pack(pop)

	static void Free(char* p)
	{
		switch (static_cast<Type>(*p))
		{
		case Type::kTest:
			delete reinterpret_cast<packet::Test*>(p);
			break;
		case Type::kSCPosition:
			delete reinterpret_cast<packet::SCPosition*>(p);
			break;
		case Type::kSCNewEntity:
			delete reinterpret_cast<packet::SCNewEntity*>(p);
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