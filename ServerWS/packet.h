//---------------------------------------------------
// 
// packet.h - 패킷 관련 구조체 및 열거자 정의
// 
//---------------------------------------------------

#pragma once
#include <print>
#include <memory>
#include "free_list.h"
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
		void Reset(char*&) const noexcept {};
		Size size{};
		Type type{};
	};

	template<class Packet>
	inline constexpr Size GetPacketSize() noexcept
	{
		return sizeof(Packet) - sizeof(Base);
	}

	template<class Packet>
	void Deserialize(Packet* packet, char*& byte) noexcept
	{
		memcpy(((char*)packet) + sizeof(Base), byte, GetPacketSize<Packet>());
		byte += GetPacketSize<Packet>();
	}

	struct Test : Base {
		Test() noexcept : Base{ GetPacketSize<decltype(*this)>(), Type::kTest },
			a{ 7 }, b{ 8 }, c{ 9 } {
		}

		Test(int a, int b, int c) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kTest }, a{ a }, b{ b }, c{ c } {}

		Test(char*& byte) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kTest }, a{}, b{}, c{} {
			Deserialize(this, byte);
		}

		void Reset(int rs_a, int rs_b, int rs_c) noexcept {
			a = rs_a;
			b = rs_b;
			c = rs_c;
		}

		int a, b, c;
	};

	struct SCPosition : Base {
		SCPosition() noexcept : Base{ GetPacketSize<decltype(*this)>(), Type::kSCPosition },
			id{}, x{}, y{}, z{} {}

		SCPosition(entity::ID id, float x, float y, float z) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCPosition },
			id(id), x{ x }, y{ y }, z{ z } {}

		void Reset(entity::ID rs_id, float rs_x, float rs_y, float rs_z) noexcept {
			id = rs_id;
			x = rs_x;
			y = rs_y;
			z = rs_z;
		}


		entity::ID id;
		float x;
		float y;
		float z;
	};

	struct SCNewEntity : SCPosition {
		SCNewEntity(entity::ID id, float x, float y, float z, entity::Type et) noexcept
			: SCPosition{ id, x, y, z }, entity_type{ et } {
			type = Type::kSCNewEntity;
			size = GetPacketSize<decltype(*this)>();
		}
		void Reset(entity::ID rs_id, float rs_x, float rs_y, float rs_z, entity::Type rs_et) noexcept {
			id = rs_id;
			x = rs_x;
			y = rs_y;
			z = rs_z;
			entity_type = rs_et;
		}
		entity::Type entity_type;
	};

	struct SCRemoveEntity : Base {
		SCRemoveEntity() noexcept : Base{ GetPacketSize<decltype(*this)>(), Type::kSCRemoveEntity },
			id{} {}

		SCRemoveEntity(entity::ID id) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCRemoveEntity }, id{ id } {}

		void Reset(entity::ID rs_id) noexcept {
			id = rs_id;
		}

		entity::ID id;
	};

	struct SCResult : Base {
		SCResult(bool value, char flag = 0) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCResult }, data{} {
			data |= (static_cast<char>(value) << 7);
			data |= flag;
		}
		void Reset(bool value, char flag = 0) noexcept {
			data |= (static_cast<char>(value) << 7);
			data |= flag;
		}

		char data;
	};

	struct SCCheckConnection : Base {
		SCCheckConnection() noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCCheckConnection }, value{ 0x12 } {}
		void Reset() noexcept {}
		char value;
	};

	struct CSJoinParty : Base {
		/*CSJoinParty(int id)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSJoinParty }, id(id) {}*/
		CSJoinParty(char*& byte) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSJoinParty }, id{} {
			Deserialize(this, byte);
		}
		unsigned short id;
	};

	struct CSLeaveParty : Base {
		CSLeaveParty() noexcept : Base{ GetPacketSize<decltype(*this)>(), Type::kCSLeaveParty } {}
	};

	struct CSPosition : Base {
		CSPosition(char*& byte) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSPosition }, x{}, y{}, z{} {
			Deserialize(this, byte);
		}

		float x;
		float y;
		float z;
	};

#pragma pack(pop)

	inline void Free(void* p) noexcept
	{
		switch (*reinterpret_cast<Type*>(p))
		{
		case Type::kTest: {
			using Packet = Test;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		case Type::kSCPosition: {
			using Packet = SCPosition;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		case Type::kSCNewEntity: {
			using Packet = SCNewEntity;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		default:
			break;
		}
	}

	inline std::string CheckBytes(char* bytes, int size) noexcept
	{
		std::string data;
		for (int i = 0; i < size; ++i) {
			data += std::format("{:02X} ", (unsigned int)(*(unsigned char*)(bytes + i)));
		}
		return std::format("send {} bytes: {}", size, data);
	}
}