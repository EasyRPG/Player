#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <type_traits>

#include "../lua/lua.hpp"
#include "../ldebug.hpp"
#include "ulexception2.hpp"
#include "lua_stack.hpp"
#include "dispatcher.hpp"

namespace leasy::ul2 {
  
  /**
   * @class lstate
   * @brief Lua VM babysitter.
   *
   * Wraps a lua_State so you don't accidentally summon demons
   * (you still can, just slightly harder now).
   * If something breaks, it throws. Aggressively.
   */
  class lstate {
    private:
    lua_State *L;

    private:
    
    /**
     * @brief Splits a dotted string into pieces.
     *
     * Example:
     *  "a.b.c" -> ["a", "b", "c"]
     *
     * @param name The thing you typed (hopefully valid)
     * @return Vector of parts
     *
     * @throw ulexception2 if:
     *  - empty string (why)
     *  - empty segment ("a..b" congrats, you broke it)
     */
    inline std::vector<std::string> split(const std::string &name) const {
      std::vector<std::string> out;
      std::stringstream ss(name);
      std::string item;
      
      while (std::getline(ss, item, '.')) {
        if (item.empty())
          ulthrow("Malformed name (empty segment)");
        out.push_back(item);
      }
      
      if (out.empty())
        ulthrow("Empty name");

      return out;
    }

    /**
     * @brief Walks through Lua tables like it's an RPG dungeon.
     *
     * Starts at _G and follows the path step by step.
     *
     * @param parts Pre-split path (don't mess it up)
     * @param leave_parent Stop before the last element (used for setting stuff)
     * @param create_missing Stop being stupid and yell at ya when parents ain't exist lol.
     *
     * @throw ulexception2 if something doesn't exist (rip)
     *
     * @note Stack after:
     *  - top = final value (or parent if leave_parent = true)
     */
    inline void push_path(
      const std::vector<std::string> &parts,
      bool leave_parent = false,
      bool create_missing = true
    ) {
      lua_getglobal(L, "_G");

      for (size_t i = 0; i < parts.size(); ++i) {
        const bool last = (i == parts.size() - 1);

        if (last && leave_parent)
          return;

        lua_getfield(L, -1, parts[i].c_str());

        if (lua_isnil(L, -1)) {
          if (!create_missing) ulthrow(std::string("Invalid path: ") + parts[i]);

          lua_pop(L, 1);
          lua_newtable(L);

          lua_pushvalue(L, -1);
          lua_setfield(L, -3, parts[i].c_str());
        }

        lua_remove(L, -2);
      }
    }

  public:

    /**
     * @brief Creates a Lua state.
     *
     * @param libs Load standard libs (recommended unless you enjoy suffering)
     */
    inline lstate(bool libs = true) : L(luaL_newstate()) {
      if (libs) luaL_openlibs(L);
    }

    /**
     * @brief Destroys the Lua state.
     *
     * Goodbye Lua. You did your best.
     */
    inline ~lstate() { lua_close(L); }

    /**
     * @brief Pushes a value into Lua.
     *
     * Works with:
     *  - "var" -> global
     *  - "a.b.c" -> nested field
     *
     * @tparam T Type (must be supported by lua_stack or be a C function)
     * @param name Where to put it
     * @param value The thing you're shoving into Lua
     *
     * @throw ulexception2 if:
     *  - path is cursed
     *  - parent isn't a table (seriously why)
     */
    template <typename T>
    inline void push(const std::string &name, const T &value) {
      auto parts = split(name);

      if (parts.size() == 1) {
        push_value(value);
        lua_setglobal(L, parts[0].c_str());
        return;
      }

      push_path(parts, true, true);

      if (!lua_istable(L, -1))
        ulthrow("Target parent is not a table");

      push_value(value);
      lua_setfield(L, -2, parts.back().c_str());

      lua_pop(L, 1);
    }

    /**
     * @brief Binds a C++ function into Lua. (It's better than the other one)
     *
     * Basically black magic:
     *  - takes a normal C++ function
     *  - runs it through your weird dispatch trampoline
     *  - Lua thinks it's a regular function (it isn't)
     *
     * @tparam F Function thingy (known at compile time, don't get fancy)
     * @param name Where to expose it in Lua
     */
    template <auto F>
    inline void bind(const std::string &name) {
      auto parts = split(name);

      if (parts.size() == 1) {
        lua_pushcfunction(L, dispatch<F>);
        lua_setglobal(L, parts[0].c_str());
        return;
      }

      push_path(parts, true);

      if (!lua_istable(L, -1))
        ulthrow("Target parent is not a table (bind)");

      lua_pushcfunction(L, dispatch<F>);
      lua_setfield(L, -2, parts.back().c_str());

      lua_pop(L, 1);
    }

  private:

    /**
     * @brief Push helper (generic version).
     *
     * Uses lua_stack for normal types.
     */
    template <typename T>
    inline void push_value(const T &value) {
      lua_stack<T>::push(L, value);
    }

    /**
     * @brief Push helper (C function specialization).
     *
     * Because Lua likes raw C functions.
     */
    inline void push_value(lua_CFunction fn) {
      lua_pushcfunction(L, fn);
    }

  public:

    /**
     * @brief Calls a Lua function (ignores return value).
     *
     * @param name Function path
     * @param args Stuff to throw at it
     */
    template <typename ... Args>
    inline void call(const std::string &name, Args&&... args) {
      call<void>(name, std::forward<Args>(args)...);
    }

    /**
     * @brief Calls a Lua function and expects something back.
     *
     * @tparam R Return type
     * @param name Function path
     * @param args Arguments
     *
     * @return Whatever Lua felt like returning
     *
     * @throw ulexception2 if:
     *  - not a function (oops)
     *  - Lua explodes (common)
     *  - no return value (rude)
     */
    template <typename R = void, typename ... Args>
    inline R call(const std::string &name, Args&&... args) {
      auto parts = split(name);

      push_path(parts, false, false);

      if (!lua_isfunction(L, -1))
        ulthrow("Target is not a function: " + name);

      (push_value(std::forward<Args>(args)), ...);

      constexpr int retc = std::is_void_v<R> ? 0 : 1;

      if (lua_pcall(L, sizeof...(Args), retc, 0) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        std::string msg = err ? err : "Unknown Lua error";
        lua_pop(L, 1);
        ulthrow(msg);
      }

      if constexpr (std::is_void_v<R>) {
        return;
      } else {
        if (lua_gettop(L) < 1)
          ulthrow("Expected return value");

        R result = lua_stack<R>::get(L, -1);
        lua_pop(L, 1);
        return result;
      }
    }

    /** 
     * @brief Executes a string. Yes, literally.
     *
     * If Lua cries, we throw.
     */
    inline void dostring(const std::string &string) {
      if (luaL_dostring(L, string.c_str()) != LUA_OK) {
        auto S__ = lua_tostring(L, -1);
        std::string msg = "lua dostring error:\t" + std::string(S__ ? S__ : "<unknown>") + "\nwith string:\"" + string + "\"";
        ulthrow(msg);
      }
    }

    /** 
     * @brief Executes a file. Revolutionary concept.
     *
     * Same idea as dostring, but with files (wild, I know).
     */
    inline void dofile(const std::string &string) {
      if (luaL_dofile(L, string.c_str()) != LUA_OK) {
        auto S__ = lua_tostring(L, -1);
        std::string msg = "lua dofile error: " + std::string(S__ ? S__ : "<unknown>");
        ulthrow(msg);
      }
    }
  };

}