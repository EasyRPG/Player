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

#include "ulexception2.hpp"
#include "function_traits.hpp"
#include "leasy/typings/metatypes.hpp"
#include "leasy/kits/variant.hpp"
#include "../kits/cppsupport/bases.hpp"

#include <any>
#include <tuple>
#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <fmt/format.h>

#include "leasy/ily3/basetypes.hpp"
#include "leasy/metadata/type.hpp"

namespace leasy::ul2 {
  template<typename T, typename = void>
  struct lua_stack {
    static bool accepts(lua_State *, int) {
      return false;
    }
  };

  struct function_holder {
    // damnit, it's for yk destroying idk what func when lua can't blabla
    std::function<int(lua_State *)> fn; // I knew lua was boring sometimes... Anyways, it's cute..
  };

  inline static int function_dispatch(lua_State *L) {
    auto *holder = static_cast<function_holder *>(
      lua_touserdata(L, lua_upvalueindex(1))
    );

    return holder->fn(L);
  }

  inline static int function_gc(lua_State *L) {
    auto *holder = static_cast<function_holder *>(lua_touserdata(L, 1));
    holder->~function_holder();
    return 0;
  }

  template<typename R, typename Tuple, typename Callable, size_t... I>
  static int invoke_stdfn(const Callable &fn, lua_State *L, std::index_sequence<I...>) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(fn, lua_stack<std::tuple_element_t<I, Tuple> >::get(L, I + 1)...);
      return 0;
    } else {
      auto r = std::invoke(fn, lua_stack<std::tuple_element_t<I, Tuple> >::get(L, I + 1)...);
      return lua_stack<R>::push(L, r);
    }
  }

  inline void push_function2(lua_State *L, std::function<int(lua_State *)> fn) {
    auto *holder = static_cast<function_holder *>(
      lua_newuserdatauv(L, sizeof(function_holder), 0)
    );
    new(holder) function_holder{std::move(fn)};

    luaL_getmetatable(L, "__leasy_function");
    lua_setmetatable(L, -2);

    lua_pushcclosure(L, function_dispatch, 1);
  }

  /*
   * Integral types
   */
  template<typename T>
  struct lua_stack<
        T,
        std::enable_if_t<
          std::is_integral_v<T> &&
          !std::is_same_v<T, bool>
        >
      > {
    static T get(lua_State *L, int index) {
      return static_cast<T>(
        luaL_checkinteger(L, index)
      );
    }

    static bool accepts(lua_State *L, int index) {
      return lua_isinteger(L, index);
    }

    static int push(lua_State *L, T value) {
      lua_pushinteger(
        L,
        static_cast<lua_Integer>(value)
      );
      return 1;
    }
  };


  /*
   * Boolean
   */
  template<>
  struct lua_stack<bool> {
    static bool get(lua_State *L, int index) {
      return lua_toboolean(L, index);
    }

    static bool accepts(lua_State *L, int index) {
      return lua_isboolean(L, index);
    }

    static int push(lua_State *L, bool value) {
      lua_pushboolean(L, value);
      return 1;
    }
  };


  /*
   * Floating point
   */
  template<typename T>
  struct lua_stack<
        T,
        std::enable_if_t<std::is_floating_point_v<T>>
      > {
    static T get(lua_State *L, int index) {
      return static_cast<T>(
        luaL_checknumber(L, index)
      );
    }

    static bool accepts(lua_State *L, int index) {
      return lua_isnumber(L, index);
    }

    static int push(lua_State *L, T value) {
      lua_pushnumber(
        L,
        static_cast<lua_Number>(value)
      );

      return 1;
    }
  };


  /*
   * std::string
   */
  template<>
  struct lua_stack<std::string> {
    static std::string get(lua_State *L, int index) {
      size_t len;

      const char *str = luaL_checklstring(L, index, &len);

      return std::string(str, len);
    }

    static bool accepts(lua_State *L, int index) {
      return lua_isstring(L, index);
    }

    static int push(lua_State *L, const std::string &value) {
      lua_pushlstring(
        L,
        value.data(),
        value.size()
      );

      return 1;
    }
  };


  /*
   * C strings
   */
  template<>
  struct lua_stack<const char *> {
    static const char *get(lua_State *L, int index) {
      return luaL_checkstring(L, index);
    }

    static bool accepts(lua_State *L, int index) {
      return lua_isstring(L, index);
    }

    static int push(lua_State *L, const char *value) {
      lua_pushstring(L, value);
      return 1;
    }
  };


  /*
   * Userdata system
   */

  struct ul2userdata {
    std::shared_ptr<void> data;
    std::shared_ptr<metadata::Class> type;

    template <typename T>
    decltype(auto) cast() const {
      using U = kits::remove_shared_ptr_t<fmt::remove_cvref_t<T>>;

      if (! type->isAssignableTo<U>()) {
        ulthrow(
            "casting type " +
            type->fullname() +
            " to " +
            typeidof<U>()->fullname() +
            " (impossible cast!)"
        );
      }

      if constexpr (std::is_pointer_v<T>) {
        using P = std::remove_pointer_t<T>;
        return static_cast<P *>(data.get());
      } else {
        using R = std::remove_reference_t<T>;
        using V = std::remove_cv_t<R>;

        auto *ptr = static_cast<V *>(data.get());
        return static_cast<R &>(*ptr);
      }
    }

    template <typename T>
    static int lookup(lua_State *L) {
      static auto type = metadata::typeidof<T>();
      auto method_name = luaL_checkstring(L, 2);
      auto method = type->get_method(method_name);

      if (method) {
        push_function2(L, method->lua());
        return 1;
      }

      return luaL_error(L, "method '%s' not found in base %s (%s)", method_name, type->fullname().c_str(),
                          type->cindex().name());
    }

    template<typename T>
    std::shared_ptr<T> castShared() const {
      using U = fmt::remove_cvref_t<T>;

      if (!type->isAssignableTo<U>()) {
        ulthrow(
            "invalid cast from " +
            type->fullname() +
            " to " +
            typeidof<U>()->fullname()
        );
      }

      return std::shared_ptr<U>(
        data,
        static_cast<U*>(data.get())
      );
    }
  };

  template<typename T>
  struct lua_stack<
        T,
        std::enable_if_t<
          !std::is_arithmetic_v<T> &&
          !std::is_same_v<T, std::string> &&
          !std::is_same_v<T, const char *> &&
          !kits::is_vector_v<T> &&
          !kits::is_unordered_map_v<T> &&
          !kits::is_shared_ptr_v<T> &&
          !kits::is_tuple_v<T> &&
          !kits::is_optional_v<T>
        >
      > {

    static bool accepts(lua_State *L, int index) {
      return lua_isuserdata(L, index) &&
        static_cast<ul2userdata*>(lua_touserdata(L, index))->type->isAssignableFrom<T>();
    }

    static T get(lua_State *L, int index) {
      if constexpr (typings2::can_store_by_value<T>) {
        if (! lua_isuserdata(L, index)) {
          ulthrow("expected userdata!");
        }

        auto *ptr = static_cast<ul2userdata *>(lua_touserdata(L, index));
        return ptr->cast<T>();
      } else {
        ulthrow("cannot retrieve a value because its type is incomplete or abstract (which is illegal in C++)");
      }
    }

    static int push(lua_State *L, const T &value) {
      if constexpr (typings2::can_store_by_value<T>) {
        using U = fmt::remove_cvref_t<T>;
        void *memory = lua_newuserdata(L, sizeof(ul2userdata));
        new(memory) ul2userdata{
          .data = std::make_shared<U>(value),
          .type = typeidof<U>()
        };

        if (luaL_newmetatable(L, typings2::leasy_typeid<U>().name())) {
          lua_pushcfunction(L, [](lua_State* L) -> int {
            auto* ptr = static_cast<ul2userdata*>(
              luaL_checkudata(L, 1, typings2::leasy_typeid<U>().name())
            );

            ptr->~ul2userdata();
            return 0;
          });
          lua_setfield(L, -2, "__gc");

          lua_pushcfunction(L, &ul2userdata::lookup<T>);
          lua_setfield(L, -2, "__index");
        }

        lua_setmetatable(L, -2);
        return 1;
      } else {
        // This type cannot be stored directly inside Lua userdata.
        //
        // Reasons include:
        //   - T is incomplete (sizeof(T) is unknown)
        //   - T is abstract (cannot be instantiated)
        //
        // Intentionally do nothing so the code still compiles.
        // The library may emit a warning here in the future.
        (void) L;
        (void) value;
      }

      return 0;
    }
  };

  namespace detail {
    template<typename T>
    bool try_get(lua_State *L, int idx, T &out) {
      if (!lua_stack<T>::accepts(L, idx))
        return false;

      out = lua_stack<T>::get(L, idx);
      return true;
    }
  }

  template<typename Variant, typename T, typename... Rest>
  struct variant_loader {
    static bool load(lua_State *L, int idx, Variant &v) {
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
  struct lua_stack<leasy::kits::variant<Ts...> > {
    using variant_t = leasy::kits::variant<Ts...>;

    static bool accepts(lua_State *L, int index) {
      return (lua_stack<Ts>::accepts(L, index) || ...);
    }

    static variant_t get(lua_State *L, int index) {
      variant_t result;

      if (!variant_loader<variant_t, Ts...>::load(L, index, result)) luaL_error(L, "No matching variant alternative.");

      return result;
    }

    static int push(lua_State *L, const variant_t &v) {
      std::visit(
        [&](auto const &x) {
          lua_stack<std::decay_t<decltype(x)> >::push(L, x);
        },
        v.internal()
      );
      return 1;
    }
  };

  template<typename... Ts>
  struct lua_stack<std::tuple<Ts...> > {
  private:
    template<std::size_t... I>
    static std::tuple<Ts...> get_impl(lua_State *L,
                                      int index,
                                      std::index_sequence<I...>) {
      return {
        lua_stack<std::tuple_element_t<I, std::tuple<Ts...> > >::get(
          L,
          index + static_cast<int>(I)
        )...
      };
    }

    template<std::size_t... I>
    static void push_impl(lua_State *L,
                          const std::tuple<Ts...> &t,
                          std::index_sequence<I...>) {
      (
        lua_stack<
          std::tuple_element_t<I, std::tuple<Ts...> >
        >::push(L, std::get<I>(t)),
        ...
      );
    }

    template<std::size_t... I>
    static bool accepts_impl(
        lua_State* L,
        int index,
        std::index_sequence<I...>
    ) {
      return (
          lua_stack<std::tuple_element_t<I, std::tuple<Ts...>>>::accepts(
              L,
              index + static_cast<int>(I)
          ) && ...
      );
    }

  public:
    static bool accepts(lua_State* L, int index) {
      return accepts_impl(
          L,
          index,
          std::index_sequence_for<Ts...>{}
      );
    }

    static std::tuple<Ts...> get(lua_State *L, int index) {
      return get_impl(
        L,
        index,
        std::index_sequence_for<Ts...>{}
      );
    }

    static int push(lua_State *L,
                    const std::tuple<Ts...> &t) {
      push_impl(
        L,
        t,
        std::index_sequence_for<Ts...>{}
      );

      return sizeof...(Ts);
    }
  };

  template<typename T>
  struct lua_stack<std::vector<T> > {
    static std::vector<T> get(lua_State *L, int index) {
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

    static int push(lua_State *L, const std::vector<T> &vec) {
      lua_createtable(L,
                      static_cast<int>(vec.size()),
                      0);

      int i = 1;

      for (auto const &e: vec) {
        lua_stack<T>::push(L, e);
        lua_seti(L, -2, i++);
      }

      return 1;
    }

    static bool accepts(lua_State *L, int index) {
      return lua_istable(L, index);
    }
  };

  template<typename K, typename V>
  struct lua_stack<std::unordered_map<K, V> > {
    static bool accepts(lua_State *L, int index) {
      return lua_istable(L, index);
    }

    static std::unordered_map<K, V> get(lua_State *L, int index) {
      luaL_checktype(L, index, LUA_TTABLE);

      std::unordered_map<K, V> map;

      lua_pushnil(L);

      while (lua_next(L, index)) {
        K key = lua_stack<K>::get(L, -2);
        V value = lua_stack<V>::get(L, -1);

        map.emplace(std::move(key),
                    std::move(value));

        lua_pop(L, 1);
      }

      return map;
    }

    static int push(
      lua_State *L,
      const std::unordered_map<K, V> &map) {
      lua_newtable(L);

      for (auto const &[k, v]: map) {
        lua_stack<K>::push(L, k);
        lua_stack<V>::push(L, v);

        lua_settable(L, -3);
      }

      return 1;
    }
  };

  template<typename T>
  struct lua_stack<std::optional<T> > {
    using U = std::optional<T>;

    static U get(lua_State *L, int index) {
      if (lua_isnoneornil(L, index))
        return std::nullopt;

      return lua_stack<T>::get(L, index);
    }

    static bool accepts(lua_State *L, int index) {
      return lua_isnoneornil(L, index) || lua_stack<T>::accepts(L, index);
    }

    static int push(lua_State *L, const U &value) {
      if (!value) {
        lua_pushnil(L);
        return 1;
      }

      return lua_stack<T>::push(L, *value);
    }
  };

  template<typename T>
  struct lua_stack<std::shared_ptr<T> > {
    using ptr_t = std::shared_ptr<T>;

    static bool accepts(lua_State *L, int index) {
      return lua_isuserdata(L, index) &&
        static_cast<ul2userdata*>(lua_touserdata(L, index))->type->isAssignableFrom<T>();
    }

    static ptr_t get(lua_State *L, int index) {
      if (! lua_isuserdata(L, index)) {
        ulthrow("expected userdata!");
      }

      auto ud = static_cast<ul2userdata*>(lua_touserdata(L, index));
      return ud->castShared<T>();
    }

    static int push(lua_State *L, ptr_t value) {
      void *mem = lua_newuserdata(L, sizeof(ul2userdata));

      new(mem) ul2userdata{
        .data = std::const_pointer_cast<std::remove_const_t<T>>(std::move(value)),
        .type = typeidof<T>()
      };

      if (luaL_newmetatable(L, typings2::leasy_typeid<T>().name())) {
        lua_pushcfunction(L, [](lua_State* L){
          auto* ptr = static_cast<ul2userdata*>(luaL_checkudata(L,1,typings2::leasy_typeid<T>().name()));

          ptr->~ul2userdata();
          return 0;
        });

        lua_setfield(L, -2, "__gc");
      }

      lua_setmetatable(L, -2);
      return 1;
    }
  };

  inline std::string lua_type_name(lua_State* L, int index) {
    if (lua_isuserdata(L, index)) {
      if (lua_getmetatable(L, index)) {
        lua_getfield(L, -1, "__name");

        if (lua_isstring(L, -1)) {
          std::string name = lua_tostring(L, -1);
          lua_pop(L, 2);
          return name;
        }

        lua_pop(L, 2);
      }

      return "userdata";
    }

    return luaL_typename(L, index);
  }

  template<typename T, size_t I>
  std::pair<bool, std::string> accepts_fr(lua_State *L, int index) {
    auto ac = lua_stack<T>::accepts(L, index);
    if (!ac) {
      return {
        ac,
        "argument #" + std::to_string(I) + ": expected " + std::string(nameof<T>()) + " got: " + lua_type_name(L, index),
      };
    }

    return {ac, ""};
  }

  template<typename Tuple, std::size_t... I>
  std::pair<bool, std::stack<std::string> >
  matches_impl(
    lua_State *L,
    std::index_sequence<I...>) {
    std::stack<std::string> reasons;
    bool matched = true;

    (
      [&] {
        auto [ok, reason] = accepts_fr<
          std::tuple_element_t<I, Tuple>, I + 1
        >(L, I + 1);

        if (!ok) {
          matched = false;
          reasons.push(std::move(reason));
        }
      }(),
      ...
    );

    return {matched, std::move(reasons)};
  }

  template<typename F>
  std::pair<bool, std::stack<std::string> > matches(lua_State *L) {
    using traits = function_traits<F>;
    using args = typename traits::args_tuple;

    constexpr std::size_t N = std::tuple_size_v<args>;

    if (lua_gettop(L) != N) {
      std::stack<std::string> s;
      s.emplace(fmt::format("expected {} arguments, got {}!", N, lua_gettop(L)));
      return {false, s};
    }

    return matches_impl<args>(L, std::make_index_sequence<N>{});
  }
} // namespace leasy::ul2
