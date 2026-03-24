#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "function_traits.hpp"
#include "lua_stack.hpp"
#include "types.hpp"

namespace leasy::ul2 {

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

template <auto F> int dispatch(lua_State *L) {
  using traits = function_traits<decltype(F)>;

  using R = typename traits::return_type;
  using Tuple = typename traits::args_tuple;

  constexpr std::size_t N = traits::arity;

  return call_impl<F, R, Tuple>(L, std::make_index_sequence<N>{});
}

} // namespace leasy::ul2
