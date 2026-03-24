#pragma once

#include "../lua/lua.hpp"
#include "ulexception2.hpp"

#include <any>
#include <tuple>
#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>

namespace leasy::ul2 {

template <typename T, typename = void> struct lua_stack;

/** @brief Weirdness for brinding your type to Lua. Please don't mess with this up or ima js beat u guys up */
template <typename T>
struct lua_stack<
    T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
  static T get(lua_State *L, int index) {
    return static_cast<T>(luaL_checkinteger(L, index));
  }

  static void push(lua_State *L, T value) {
    lua_pushinteger(L, static_cast<lua_Integer>(value));
  }
};

template <> struct lua_stack<bool> {
  static bool get(lua_State *L, int index) { return lua_toboolean(L, index); }

  static void push(lua_State *L, bool value) { lua_pushboolean(L, value); }
};

template <typename T>
struct lua_stack<T, std::enable_if_t<std::is_floating_point_v<T>>> {
  static T get(lua_State *L, int index) {
    return static_cast<T>(luaL_checknumber(L, index));
  }

  static void push(lua_State *L, T value) {
    lua_pushnumber(L, static_cast<lua_Number>(value));
  }
};

template <> struct lua_stack<std::string> {
  static std::string get(lua_State *L, int index) {
    size_t len;
    const char *str = luaL_checklstring(L, index, &len);
    return std::string(str, len);
  }

  static void push(lua_State *L, const std::string &value) {
    lua_pushlstring(L, value.data(), value.size());
  }
};

template <> struct lua_stack<const char *> {
  static const char *get(lua_State *L, int index) {
    return luaL_checkstring(L, index);
  }

  static void push(lua_State *L, const char *value) {
    lua_pushstring(L, value);
  }
};

using bridged_types = std::tuple<
  bool,
  char, short, int, long, long long,
  unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long,
  float, double, long double,
  const char*, std::string
>;

template <> struct lua_stack<std::any> {

  static std::any get(lua_State *L, int index) {
    switch (lua_type(L, index)) { 
      case LUA_TBOOLEAN:
        return static_cast<bool>(lua_toboolean(L, index));

      case LUA_TNUMBER:
        if (lua_isinteger(L, index))
          return static_cast<lua_Integer>(lua_tointeger(L, index));
        else
          return static_cast<lua_Number>(lua_tonumber(L, index));

      case LUA_TSTRING: {
        size_t len;
        const char* str = lua_tolstring(L, index, &len);
        return std::string(str, len);
      }

      case LUA_TNIL:
        return std::any{};

      default:
        ulthrow("Unsupported Lua type for std::any");
        return {};
    }
  }

private:

  template <typename T>
  static bool try_push(lua_State* L, const std::any& value) {
    if (value.type() == typeid(T)) {
      lua_stack<T>::push(L, std::any_cast<T>(value));
      return true;
    }
    return false;
  }

  template <typename Tuple, std::size_t... I>
  static bool push_from_tuple(lua_State* L, const std::any& value, std::index_sequence<I...>) {
    return (try_push<std::tuple_element_t<I, Tuple>>(L, value) || ...);
  }

public:

  static void push(lua_State* L, const std::any& value) {

    if (!value.has_value()) {
      lua_pushnil(L);
      return;
    }

    bool handled = push_from_tuple<bridged_types>(
      L, value,
      std::make_index_sequence<std::tuple_size_v<bridged_types>>{}
    );

    if (!handled)
      ulthrow( "Unsupported std::any type");
  }
};

} // namespace leasy::ul2
