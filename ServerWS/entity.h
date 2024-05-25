//---------------------------------------------------
// 
// entity.h - 엔티티 관련 클래스의 부모 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include "coord.h"
#include "world_map.h"

namespace entity {
	enum class Type : unsigned char {
		kNone, kPlayer, kMob, kBoss
	};
	using ID = unsigned short;
	constexpr inline ID kPartnerID{ 0xFFFF };
	constexpr inline ID kAvatarID{ 0xFFFE };

	class Base {
	public:
		Base(ID id, float x, float y, float z, short hp) noexcept
			: id_(id), pos_{ x, y, z }, hp_{ hp }, type_{}, dir_{}, flag_{} {
			region_ = world_map.FindRegion(pos_);
		}

		void Reset(ID rs_id, float rs_x, float rs_y, float rs_z, short hp) {
			id_ = rs_id;
			SetPosition(rs_x, rs_y, rs_z);
			hp_ = hp;
			flag_ = 0;
		}

		void SetPosition(float x, float y, float z) noexcept {
			float xy[2]{ x, y };
			*reinterpret_cast<long long*>(&pos_.x) = *reinterpret_cast<long long*>(xy);
			pos_.z = z;
		}

		ID GetID() const noexcept {	return id_; }

		const Position& GetPostion() const noexcept { return pos_; }

		void Delete() noexcept {}

		auto GetType() const noexcept {
			return type_;
		}

		char GetFlag() const noexcept;
		float GetVel() const noexcept;

		char flag_;
		int region_;
		float dir_;
	protected:
		void SetType(Type t) noexcept {
			type_ = t;
		}
	private:
		ID id_;
		short hp_;
		Type type_;
		Position pos_;
	};
}