#include "session.h"
#include "view_list.h"

namespace client {
	void Session::Delete() noexcept {
		if (debug::DisplaysMSG()) {
			std::print("[Info] (ID: {}) has left the game.\n", GetID());
		}

		delete view_lists[id_];
		view_lists[id_] = new ViewList{ 1 };
	}
}