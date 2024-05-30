//---------------------------------------------------
// 
// player.h - �÷��̾� ��ƼƼ Ŭ���� ����
// 
//---------------------------------------------------

#pragma once
#include "world_map.h"
#include "entity.h"

namespace entity {
	class Player : public Base {
	public:
		Player() : Base{ 0, 0.0f, 0.0f, 0.0f, 100 } {
			SetType(Type::kPlayer);
		}
		int SetPosition(float x, float y, float z) {
			Base::SetPosition(x, y, z);
			int region = world_map.FindRegion(Position{ x, y, z });
			region_ = region;
			return region;
		}
	};
}