#pragma once
#include <string>
#include "lua/include/lua.hpp"
#pragma comment (lib, "lua/lua54.lib")

namespace lua {
	class Manager {
	public:
		Manager() noexcept : ls{ luaL_newstate() } {
			luaL_openlibs(ls);
		}
		~Manager() noexcept {
			lua_close(ls);
		}
		auto GetLuaState() const noexcept {
			return ls;
		}
	private:
		lua_State* ls;
	};
	
	inline Manager manager;

	class Script {
	public:
		Script(const std::string& file_name) : ls{ manager.GetLuaState() } {
			luaL_loadfile(ls, file_name.c_str());
		}
		template<class T>
		T GetGlobalVar(const std::string& var_name) {
			lua_pcall(ls, 0, 0, 0);
			lua_getglobal(ls, var_name.c_str());
			auto val = static_cast<T>(lua_tonumber(ls, -1));
			lua_pop(ls, 1);
			return val;
		}
	private:
		lua_State* ls;
	};

	inline Script server_settings{ "server_settings.lua" };
}