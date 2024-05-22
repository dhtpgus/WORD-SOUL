#pragma once
#include <string>
#include <memory>
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
		template<class T>
		using ArgList = std::initializer_list<T>;

		Script(const std::string& file_name) noexcept : ls_{ manager.GetLuaState() } {
			luaL_loadfile(ls_, file_name.c_str());
		}
		template<class T>
		T GetConstant(const std::string& var_name) noexcept {
			lua_pcall(ls_, 0, 0, 0);
			lua_getglobal(ls_, var_name.c_str());
			auto val = static_cast<T>(lua_tonumber(ls_, -1));
			lua_pop(ls_, 1);
			return val;
		}

		template<class ArgType, class ReturnType>
		auto CallFunction(const std::string& func_name, int num_return_values, const ArgList<ArgType> args) noexcept {
			auto r = std::make_unique<std::vector<ReturnType>>();
			r->reserve(num_return_values);
			
			lua_pcall(ls_, 0, 0, 0);
			lua_getglobal(ls_, func_name.c_str());
			for (auto arg : args) {
				lua_pushnumber(ls_, static_cast<lua_Number>(arg));
			}
			lua_pcall(ls_, static_cast<int>(args.size()), num_return_values, 0);
			for (int i = -num_return_values; i < 0; ++i) {
				r->push_back(static_cast<ReturnType>(lua_tonumber(ls_, i)));
			}
			lua_pop(ls_, num_return_values);
			return std::move(r);
		}
	private:
		lua_State* ls_;
	};
}