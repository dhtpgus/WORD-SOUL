#pragma once
#include <atomic>
#include <compare>

namespace entity {
	using ID = unsigned long long;

	enum class Type : unsigned char {
		kPlayer, kMonster, kBoss
	};

	struct Postion {
		float x, y, z;
	};

	class Entity {
	public:
		Entity(float x, float y, float z, short hp) 
			: pos_{ x, y, z }, hp_{ hp }, id_{ next_id_.fetch_add(1) } {}

		void SetPostion(float x, float y, float z) {
			pos_.x = x;
			pos_.y = y;
			pos_.z = z;
		}

		ID GetID() {
			return id_;
		}
		const Postion& GetPostion() const {
			return pos_;
		}

		auto operator<=>(const Entity& rhs) const {
			return id_ <=> rhs.id_;
		}

	private:
		static std::atomic<ID> next_id_;
		ID id_;
		Postion pos_;
		short hp_;
	};
}