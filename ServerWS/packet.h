//---------------------------------------------------
// 
// packet.h - 패킷 관련 구조체 및 열거자 정의
// 
//---------------------------------------------------

#pragma once
#include <chrono>
#include <string>
#include "entity.h"

namespace packet {
	enum class Type : unsigned char {
		kTest,
		kSCNewEntity,
		kSCPosition,
		kSCRemoveEntity,
		kSCResult,
		kSCCheckConnection,
		kSCModifyHP,

		kCSJoinParty = 128,
		kCSPosition,
		kCSLeaveParty,

		kStressTest = 255
	};
	using Size = unsigned char;

#pragma pack(push, 1)

	struct Base {
		Base(Size size, Type type) : size{ size }, type{ type } {}
		Size size{};
		Type type{};
	};

	template<class Packet>
	inline constexpr Size GetPacketSize() noexcept
	{
		return sizeof(Packet) - sizeof(Base);
	}

	template<class Packet>
	void Deserialize(Packet* packet, const char* byte) noexcept
	{
		memcpy(packet, byte, GetPacketSize<Packet>() + sizeof(Base));
	}

	struct Test : Base {
		Test() noexcept : Base{ GetPacketSize<decltype(*this)>(), Type::kTest },
			a{ 7 }, b{ 8 }, c{ 9 } {
		}

		Test(int a, int b, int c) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kTest }, a{ a }, b{ b }, c{ c } {}

		Test(const char* byte) noexcept
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
		SCPosition() noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCPosition }
			, id{}, x{}, y{}, z{}, v{}, r{}, flag{} {}

		SCPosition(entity::ID id, float x, float y, float z, float v, float r, int flag) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCPosition }
			, id(id), x{ x }, y{ y }, z{ z }, v{ v }, r{ r }, flag{ static_cast<char>(flag)} {}

		SCPosition(entity::Base* e) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCPosition }
			, id{ e->GetID() }, x{ e->GetPosition().x }, y{ e->GetPosition().y }, z{ e->GetPosition().z }
			, v{ e->GetVel() }, r{ e->dir_ }, flag{ e->GetFlag() } {}

		void Reset(entity::ID rs_id, float rs_x, float rs_y, float rs_z,
			float rs_v, float rs_r, int rs_flag) noexcept {
			id = rs_id;
			x = rs_x;
			y = rs_y;
			z = rs_z;
			v = rs_v;
			r = rs_r;
			flag = static_cast<char>(rs_flag);
		}

		void Reset(entity::Base* e) noexcept {
			id = e->GetID();
			x = e->GetPosition().x;
			y = e->GetPosition().y;
			z = e->GetPosition().z;
			v = e->GetVel();
			r = e->dir_;
			flag = e->GetFlag();
		}

		entity::ID id;
		float x;
		float y;
		float z;
		float v;
		float r;
		char flag;
	};

	struct SCNewEntity : Base {
		SCNewEntity(entity::ID id, float x, float y, float z, entity::Type et, int flag) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCNewEntity }
			, id{ id }, x{ x }, y{ y }, z{ z }, entity_type{ et }, flag{ static_cast<char>(flag) } {}
		void Reset(entity::ID rs_id, float rs_x, float rs_y, float rs_z, entity::Type rs_et, int rs_flag) noexcept {
			id = rs_id;
			x = rs_x;
			y = rs_y;
			z = rs_z;
			entity_type = rs_et;
			flag = static_cast<char>(rs_flag);
		}
		entity::ID id;
		float x;
		float y;
		float z;
		entity::Type entity_type;
		char flag;
	};

	struct SCRemoveEntity : Base {
		SCRemoveEntity() noexcept : Base{ GetPacketSize<decltype(*this)>(), Type::kSCRemoveEntity },
			id{}, flag{} {}

		SCRemoveEntity(entity::ID id, int flag) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCRemoveEntity }, id{ id }, flag{ static_cast<char>(flag) } {}

		void Reset(entity::ID rs_id, int rs_flag) noexcept {
			id = rs_id;
			flag = static_cast<char>(rs_flag);
		}

		entity::ID id;
		char flag;
	};

	struct SCResult : Base {
		SCResult(bool value, char flags = 0) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCResult }, data{} {
			data |= static_cast<unsigned char>(value) * 128;
			data |= flags;
		}
		void Reset(bool value, char flags = 0) noexcept {
			data = 0;
			data |= static_cast<unsigned char>(value) * 128;
			data |= flags;
		}

		unsigned char data;
	};

	struct SCCheckConnection : Base {
		SCCheckConnection() noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCCheckConnection }, value{ 0x12 } {}
		void Reset() noexcept {}
		char value;
	};

	struct SCModifyHP : Base {
		SCModifyHP(entity::ID id, short hp_diff) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kSCModifyHP }, id{ id }, hp_diff{ hp_diff } {
		}
		void Reset(entity::ID rs_id, short rs_hp) noexcept {
			id = rs_id;
			hp_diff = rs_hp;
		}

		entity::ID id;
		short hp_diff;
	};

	struct CSJoinParty : Base {
		/*CSJoinParty(int id)
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSJoinParty }, id(id) {}*/
		CSJoinParty(const char* byte) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSJoinParty }, id{} {
			Deserialize(this, byte);
		}
		unsigned short id;
	};

	struct CSLeaveParty : Base {
		CSLeaveParty() noexcept : Base{ GetPacketSize<decltype(*this)>(), Type::kCSLeaveParty } {}
	};

	struct CSPosition : Base {
		CSPosition(const char* byte) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kCSPosition }
			, x{}, y{}, z{}, v{}, r{}, flag{} {
			Deserialize(this, byte);
		}

		float x;
		float y;
		float z;
		float v;
		float r;
		char flag;
	};

	// ---------------------------------------------

	struct StressTest : Base {
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = Clock::time_point;

		StressTest() noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kStressTest }
			, id{}, x{}, y{}, z{}, tp{} {}

		StressTest(entity::ID id, float x, float y, float z, TimePoint tp) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kStressTest }
			, id(id), x{ x }, y{ y }, z{ z }, tp{ tp } {}

		StressTest(entity::Base* e, TimePoint tp) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kStressTest }
			, id{ e->GetID() }, x{ e->GetPosition().x }, y{ e->GetPosition().y }
			, z{ e->GetPosition().z }, tp{ tp } {}

		StressTest(const char* byte) noexcept
			: Base{ GetPacketSize<decltype(*this)>(), Type::kStressTest }, id{}, x{}, y{}, z{}, tp{} {
			Deserialize(this, byte);
		}

		void Reset(entity::ID rs_id, float rs_x, float rs_y, float rs_z, TimePoint rs_tp) noexcept {
			id = rs_id;
			x = rs_x;
			y = rs_y;
			z = rs_z;
			tp = rs_tp;
		}

		void Reset(entity::Base* e, TimePoint rs_tp) noexcept {
			id = e->GetID();
			x = e->GetPosition().x;
			y = e->GetPosition().y;
			z = e->GetPosition().z;
			tp = rs_tp;
		}

		entity::ID id;
		float x;
		float y;
		float z;
		TimePoint tp;
	};

#pragma pack(pop)

	void Collect(void* p) noexcept;
}