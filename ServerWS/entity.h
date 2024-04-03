//---------------------------------------------------
// 
// entity.h - 엔티티 관련 클래스의 부모 클래스 정의
// 
//---------------------------------------------------

#pragma once

namespace entity {
	enum class Type : unsigned char {
		kPlayer, kMonster, kBoss
	};
	using ID = unsigned short;
	constexpr inline ID kPartnerID{ 0xFFFF };
	constexpr inline ID kAvatarID{ 0xFFFE };

	struct Postion {
		float x, y, z;
	};

	class Base {
	public:
		Base(ID id, float x, float y, float z, short hp) noexcept
			: id_(id), pos_{ x, y, z }, hp_{ hp } {}

		void SetPosition(float x, float y, float z) noexcept {
			pos_.x = x;
			pos_.y = y;
			pos_.z = z;
		}

		ID GetID() const noexcept {	return id_; }

		const Postion& GetPostion() const noexcept { return pos_; }
	private:
		ID id_;
		Postion pos_;
		short hp_;
	};
}