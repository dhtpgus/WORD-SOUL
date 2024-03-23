//---------------------------------------------------
// 
// player.h - 플레이어 엔티티 클래스 정의
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