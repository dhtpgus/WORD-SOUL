//---------------------------------------------------
// 
// entity.h - ��ƼƼ ���� Ŭ������ �θ� Ŭ���� ����
// 
//---------------------------------------------------

#pragma once

namespace entity {
	using ID = int;

	enum class Type : unsigned char {
		kPlayer, kMonster, kBoss
	};

	struct Postion {
		float x, y, z;
	};

	class Entity {
	public:
		Entity(ID id, float x, float y, float z, short hp) 
			: id_{ id }, pos_{ x, y, z }, hp_{ hp } {}

		void SetPostion(float x, float y, float z) {
			pos_.x = x;
			pos_.y = y;
			pos_.z = z;
		}

		ID GetID() const {
			return id_;
		}

		const Postion& GetPostion() const {
			return pos_;
		}

	private:
		ID id_;
		Postion pos_;
		short hp_;
	};
}