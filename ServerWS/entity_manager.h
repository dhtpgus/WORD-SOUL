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
		Manager(int enitity_num, int thread_num) noexcept
			: entities_{ enitity_num, thread_num } {
		}
		void UpdateEntityPosition(int id, float x, float y, float z) noexcept {
			if (entities_.TryAccess(id)) {
				entities_[id].SetPosition(x, y, z);
				entities_.EndAccess(id);
			}
		}
	private:
		using EntityArray = lf::Array<Base>;

		EntityArray entities_;
	};
}