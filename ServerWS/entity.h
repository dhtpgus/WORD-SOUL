#pragma once
#include <atomic>
#include "packet.h"

namespace entity {
	extern std::atomic_uint id;

	struct Postion {
		float x, y, z;
	};

	class Entity {
	public:
		Entity(float x, float y, float z, short hp) 
			: pos_{ x, y, z }, hp_{ hp }, id_{ id.fetch_add(1) } {}

		void SetPostion(float x, float y, float z) {
			pos_.x = x;
			pos_.y = y;
			pos_.z = z;
		}

		void SetPostion(packet::Position p) {
			SetPostion(p.x, p.y, p.z);
		}

		unsigned int GetID() {
			return id_;
		}
		const Postion& GetPostion() const {
			return pos_;
		}

	private:
		unsigned int id_;
		Postion pos_;
		short hp_;
	};
}