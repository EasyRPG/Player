/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A
 * 
 * **********************************************************************/

#pragma once

#include "../lua/lua.hpp"
#include "ulexception2.hpp"
#include "../typings/metatypes.hpp"
#include "../kits/variant.hpp" 
#include "function_traits.hpp"

#include <any>
#include <tuple>
#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <new>

namespace leasy::ul2 {


template <typename T, typename = void>
struct lua_stack;


/*
 * Integral types
 */
template <typename T>
struct lua_stack<
    T,
    std::enable_if_t<
      std::is_integral_v<T> &&
      !std::is_same_v<T, bool>
    >
>
{
  static T get(lua_State* L, int index)
  {
    return static_cast<T>(
      luaL_checkinteger(L, index)
    );
  }


  static void push(lua_State* L, T value)
  {
    lua_pushinteger(
      L,
      static_cast<lua_Integer>(value)
    );
  }
};



/*
 * Boolean
 */
template <>
struct lua_stack<bool>
{
  static bool get(lua_State* L, int index)
  {
    return lua_toboolean(L, index);
  }


  static void push(lua_State* L, bool value)
  {
    lua_pushboolean(L, value);
  }
};



/*
 * Floating point
 */
template <typename T>
struct lua_stack<
    T,
    std::enable_if_t<std::is_floating_point_v<T>>
>
{
  static T get(lua_State* L, int index)
  {
    return static_cast<T>(
      luaL_checknumber(L, index)
    );
  }


  static void push(lua_State* L, T value)
  {
    lua_pushnumber(
      L,
      static_cast<lua_Number>(value)
    );
  }
};



/*
 * std::string
 */
template <>
struct lua_stack<std::string>
{
  static std::string get(lua_State* L, int index)
  {
    size_t len;

    const char* str =
      luaL_checklstring(L, index, &len);

    return std::string(str, len);
  }


  static void push(lua_State* L, const std::string& value)
  {
    lua_pushlstring(
      L,
      value.data(),
      value.size()
    );
  }
};



/*
 * C strings
 */
template <>
struct lua_stack<const char*>
{
  static const char* get(lua_State* L, int index)
  {
    return luaL_checkstring(L, index);
  }


  static void push(lua_State* L, const char* value)
  {
    lua_pushstring(L, value);
  }
};




/*
 * Userdata system
 */

template <typename T, typename = void>
struct userdata_holder;

template <typename T>
struct userdata_holder<T, std::enable_if_t<typings2::is_complete_v<T>>>
{
  T value;
};

template <typename T>
struct userdata_holder<T, std::enable_if_t<!typings2::is_complete_v<T> || std::is_abstract_v<T>>>
{
  
};



template <typename T>
struct lua_stack<
    T,
    std::enable_if_t<
      !std::is_arithmetic_v<T> &&
      !std::is_same_v<T, std::string> &&
      !std::is_same_v<T, const char*>
    >
>
{
  using holder = userdata_holder<T>;


  static T& get(lua_State* L, int index)
  {
    if constexpr (typings2::can_store_by_value<T>) {

      auto* ptr =
      static_cast<holder*>(
        luaL_checkudata(
          L,
          index,
          typings2::leasy_typeid<T>().name()
        )
      );
      
      return ptr->value;
    } else {
      ulthrow("cannot retrieve a value because its type is incomplete or abstract (which is illegal in C++)");
    }
  }

  static void push(lua_State* L, const T& value) {
    if constexpr (typings2::can_store_by_value<T>) {
      void* memory = lua_newuserdata(L, sizeof(holder));
      new(memory) holder{value};

      if (luaL_newmetatable(L, typings2::leasy_typeid<T>().name())) {
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, [](lua_State* L) -> int {
          auto* ptr = static_cast<holder*>(luaL_checkudata(L, 1, typings2::leasy_typeid<T>().name()));
          ptr->~holder();
          return 0;
        });
        lua_settable(L, -3);
      }
      
      lua_setmetatable(L, -2);
    } else {
      // This type cannot be stored directly inside Lua userdata.
      //
      // Reasons include:
      //   - T is incomplete (sizeof(T) is unknown)
      //   - T is abstract (cannot be instantiated)
      //
      // Intentionally do nothing so the code still compiles.
      // The library may emit a warning here in the future.
      (void)L;
      (void)value;
    }
  }
};

  namespace detail {
    template<typename T>
    bool try_get(lua_State* L, int idx, T& out) {
      if constexpr (std::is_same_v<T, bool>) {
        if (!lua_isboolean(L, idx)) return false;
      } else if constexpr (std::is_integral_v<T>) {
        if (!lua_isinteger(L, idx)) return false;
      } else if constexpr (std::is_floating_point_v<T>) {
        if (!lua_isnumber(L, idx)) return false;
      } else if constexpr (std::is_same_v<T, std::string>) {
        if (!lua_isstring(L, idx)) return false;
      } else {
        if (!luaL_testudata(L, idx, typings2::leasy_typeid<T>().name())) return false;
      }

      out = lua_stack<T>::get(L, idx);
      return true;
    }
  }

  template<typename Variant, typename T, typename... Rest>
  struct variant_loader {
    static bool load(lua_State* L, int idx, Variant& v) {
      T value;
      if (detail::try_get(L, idx, value)) {
        v.set(std::move(value));
        return true;
      }
      if constexpr (sizeof...(Rest) > 0) return variant_loader<Variant, Rest...>::load(L, idx, v);
      return false;
    }
  };

  template<typename... Ts>
  struct lua_stack<leasy::kits::variant<Ts...>>
  {
    using variant_t = leasy::kits::variant<Ts...>;

    static variant_t get(lua_State* L, int index)
    {
      variant_t result;

      if (!variant_loader<variant_t, Ts...>::load(L, index, result)) luaL_error(L, "No matching variant alternative.");

      return result;
    }

    static void push(lua_State* L, const variant_t& v)
    {
      std::visit(
        [&](auto const& x)
        {
          lua_stack<std::decay_t<decltype(x)>>::push(L, x);
        },
        v.internal()
      );
    }
  };


template<typename T>
struct lua_stack<std::vector<T>>
{
  static std::vector<T> get(lua_State* L, int index)
  {
    luaL_checktype(L, index, LUA_TTABLE);
    std::vector<T> result;
    const size_t len = lua_rawlen(L, index);
    result.reserve(len);

    for (size_t i = 1; i <= len; ++i) {
      lua_geti(L, index, static_cast<lua_Integer>(i));
      result.push_back(lua_stack<T>::get(L, -1));
      lua_pop(L, 1);
    }

    return result;
  }

  static void push(lua_State* L, const std::vector<T>& vec)
  {
    lua_createtable(L,
                    static_cast<int>(vec.size()),
                    0);

    int i = 1;

    for (auto const& e : vec)
    {
      lua_stack<T>::push(L, e);
      lua_seti(L, -2, i++);
    }
  }
};

template<typename K, typename V>
struct lua_stack<std::unordered_map<K, V>>
{
  static std::unordered_map<K, V>
    get(lua_State* L, int index)
    {
      luaL_checktype(L, index, LUA_TTABLE);

      std::unordered_map<K, V> map;

      lua_pushnil(L);

      while (lua_next(L, index))
      {
        K key = lua_stack<K>::get(L, -2);
        V value = lua_stack<V>::get(L, -1);

        map.emplace(std::move(key),
                    std::move(value));

        lua_pop(L, 1);
      }

    return map;
  }

  static void push(
    lua_State* L,
    const std::unordered_map<K, V>& map)
  {
    lua_newtable(L);

    for (auto const& [k, v] : map)
    {
      lua_stack<K>::push(L, k);
      lua_stack<V>::push(L, v);

        lua_settable(L, -3);
    }
  }
};

template<typename T>
bool accepts(lua_State* L, int index)
{
  if constexpr (std::is_same_v<T, bool>)
    return lua_isboolean(L, index);

  else if constexpr (std::is_integral_v<T>)
    return lua_isinteger(L, index);

  else if constexpr (std::is_floating_point_v<T>)
    return lua_isnumber(L, index);

  else if constexpr (std::is_same_v<T, std::string>)
    return lua_isstring(L, index);

  else
    return luaL_testudata(
      L,
      index,
      typings2::leasy_typeid<T>().name()
    );
}

template<typename Tuple, std::size_t... I>
bool matches_impl(lua_State* L, std::index_sequence<I...>)
{
  return (
    accepts<
      std::tuple_element_t<I, Tuple>
    >(L, I + 1)
    && ...
  );
}

template<typename F>
bool matches(lua_State* L)
{
  using traits = function_traits<F>;
  using args = typename traits::args_tuple;

  constexpr std::size_t N = std::tuple_size_v<args>;

  if (lua_gettop(L) != N)
    return false;

  return matches_impl<args>(L, std::make_index_sequence<N>{});
}

} // namespace leasy::ul2