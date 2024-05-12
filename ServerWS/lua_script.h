#pragma once
#include <string>
#include "lua/include/lua.hpp"
#pragma comment (lib, "lua/lua54.lib")

namespace lua {
	class Manager {
	public:
		Manager() noexcept : ls_{ luaL_newstate() } {
			luaL_openlibs(ls_);
		}
		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;
		~Manager() noexcept {
			lua_close(ls_);
		}
		auto GetLuaState() const noexcept {
			return ls_;
		}
	private:
		lua_State* ls_;
	};
	
	inline thread_local Manager manager;

	class Script {
	public:
		Script(const std::string& file_name) : ls_{ manager.GetLuaState() } {
			luaL_loadfile(ls_, file_name.c_str());
		}
		template<class T>
		T GetGlobalVar(const std::string& var_name) {
			lua_pcall(ls_, 0, 0, 0);
			lua_getglobal(ls_, var_name.c_str());
			auto val = static_cast<T>(lua_tonumber(ls_, -1));
			lua_pop(ls_, 1);
			return val;
		}
	private:
		lua_State* ls_;
	};

	inline thread_local Script server_settings{ "scripts/server_settings.lua" };
}