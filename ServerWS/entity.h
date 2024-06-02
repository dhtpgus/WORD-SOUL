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

	enum class HitStatus : unsigned char {
		kNone, kBack, kFront
	};

	using ID = unsigned short;
	constexpr inline ID kPartnerID{ 0xFFFF };
	constexpr inline ID kAvatarID{ 0xFFFE };

	class Base {
	public:
		Base(ID id, float x, float y, float z, short hp) noexcept
			: id_(id), pos_{ x, y, z }, hp_{ hp }, type_{}, dir_{}, flag_{}
			, has_informed_to_client{} {
			region_ = world_map.FindRegion(pos_);
		}

		void Reset(ID rs_id, float rs_x, float rs_y, float rs_z, short rs_hp) {
			id_ = rs_id;
			SetPosition(rs_x, rs_y, rs_z);
			hp_ = rs_hp;
			flag_ = 0;
			has_informed_to_client = false;
		}

		void SetPosition(float x, float y, float z) noexcept {
			pos_.SetXYZ(x, y, z);
		}

		ID GetID() const noexcept {	return id_; }

		const Position& GetPosition() const noexcept { return pos_; }

		void Delete() noexcept {}

		auto GetType() const noexcept {
			return type_;
		}

		char GetFlag() const noexcept;
		float GetVel() const noexcept;
		bool GetDamaged(short damage) noexcept {
			hp_ -= damage;
			return hp_ < damage;
		}

		HitStatus IsAttacked(const Position& attacker_pos, float attacker_dir) noexcept;

		char flag_;
		int region_;
		float dir_;
		std::atomic<short> hp_;
		bool has_informed_to_client;
	protected:
		void SetType(Type t) noexcept {
			type_ = t;
		}
	private:
		ID id_;
		Type type_;
		Position pos_;
	};
}