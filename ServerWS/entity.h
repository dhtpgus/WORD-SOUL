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

	struct Postion {
		float x, y, z;
	};

	class Base {
	public:
		Base(int id, float x, float y, float z, short hp) 
			: id_{ id }, pos_{ x, y, z }, hp_{ hp } {}

		void SetPosition(float x, float y, float z) {
			pos_.x = x;
			pos_.y = y;
			pos_.z = z;
		}

		int GetID() const {	return id_; }

		const Postion& GetPostion() const { return pos_; }

	private:
		int id_;
		Postion pos_;
		short hp_;
	};
}