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
		Player() : Base{ -1, 0.0f, 0.0f, 0.0f, 0 } {}
	};
}