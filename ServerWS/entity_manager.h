//---------------------------------------------------
// 
// entity_manager.h - ��ƼƼ ������ Ŭ���� ����
// 
//---------------------------------------------------

#pragma once
#include "lf_array.h"
#include "thread.h"
#include "entity.h"
#include "player.h"

namespace entity {
	class Manager {
	public:
		Manager() = delete;
		Manager(int client_num, int thread_num)
			: entities_{ client_num * 100, thread_num } {
		}
		int AllocatePlayer() {
			return entities_.Allocate<Player>();
		}
	private:
		using EntityArray = lf::Array<Entity>;

		EntityArray entities_;
	};
}