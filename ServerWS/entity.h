//---------------------------------------------------
// 
// entity.h - 엔티티 관련 클래스의 부모 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include "coord.h"

namespace entity {
	enum class Type : unsigned char {
		kPlayer, kMonster, kBoss
	};
	using ID = unsigned short;
	constexpr inline ID kPartnerID{ 0xFFFF };
	constexpr inline ID kAvatarID{ 0xFFFE };

	class Base {
	public:
		Base(ID id, float x, float y, float z, short hp) noexcept
			: id_(id), pos_{ x, y, z }, hp_{ hp } {}

		void Reset(ID) {

		}

		void SetPosition(float x, float y, float z) noexcept {
			float xy[2]{ x, y };
			*reinterpret_cast<long long*>(&pos_.x) = *reinterpret_cast<long long*>(xy);
			pos_.z = z;
		}

		ID GetID() const noexcept {	return id_; }

		const Position& GetPostion() const noexcept { return pos_; }

		void Reset() noexcept {

		}

		void Delete() noexcept {

		}
	private:
		ID id_;
		short hp_;
		Position pos_;
	};
}