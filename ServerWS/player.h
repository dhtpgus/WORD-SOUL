//---------------------------------------------------
// 
// player.h - �÷��̾� ��ƼƼ Ŭ���� ����
// 
//---------------------------------------------------

#pragma once
#include "entity.h"

namespace entity {
	class Player : public Entity {
	public:
		Player(int id) : Entity{ id, 0.0f, 0.0f, 0.0f, 0 } {}
	};
}