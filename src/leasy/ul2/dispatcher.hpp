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

#include <tuple>
#include <vector>
#include <string>
#include <utility>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

#include "function_traits.hpp"
#include "lua_stack.hpp"
#include "types.hpp"

namespace leasy::ul2 {

  template<typename Callable, typename R, typename Tuple, size_t... I>
  int invoke(Callable &&f, lua_State *L, std::index_sequence<I...>) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, lua_stack<std::tuple_element_t<I, Tuple>>::get(L, I + 1)...);
      return 0;
    } else {
      auto r = std::invoke(f,lua_stack<std::tuple_element_t<I, Tuple>>::get(L, I + 1)...);

      lua_stack<R>::push(L, r);
      return 1;
    }
  }

  template <auto F, typename R, typename Tuple, std::size_t... I>
  int call_impl(lua_State *L, std::index_sequence<I...>) {
    if constexpr (std::is_void_v<R>) {
      F(lua_stack<remove_cvref_t<std::tuple_element_t<I, Tuple>>>::get(L, I + 1)...);
      return 0;
    } else {
      R r = F(lua_stack<remove_cvref_t<std::tuple_element_t<I, Tuple>>>::get(L, I + 1)...);

      lua_stack<R>::push(L, r);
      return 1;
    }
  }

  template<auto F>
  int dispatch(lua_State* L) {
    using traits = function_traits<decltype(F)>;

    return invoke<
      decltype(F),
      typename traits::return_type,
      typename traits::args_tuple
    >(F, L, std::make_index_sequence<traits::arity>{});
  }
}

