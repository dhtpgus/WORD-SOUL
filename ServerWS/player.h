//---------------------------------------------------
// 
// player.h - �÷��̾� ��ƼƼ Ŭ���� ����
// 
//---------------------------------------------------

#pragma once
#include "entity.h"

namespace entity {
	class Player : public Base {
	public:
		Player(int id) : Base{ id, 0.0f, 0.0f, 0.0f, 0 } {}
	};
}