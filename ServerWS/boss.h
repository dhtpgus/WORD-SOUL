//---------------------------------------------------
// 
// boss.h - ���� ��ƼƼ Ŭ���� ����
// 
//---------------------------------------------------

#pragma once
#include "entity.h"

namespace entity {
	class Boss : public Base {
		Boss() : Base{ 0, 0.0f, 0.0f, 0.0f, 1000 } {
			SetType(Type::kBoss);
		}
	};
}