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
#include <functional>

namespace leasy::ul2 {
  template<typename T>
  struct function_traits;

  template<typename R, typename... Args>
  struct function_traits<R(Args...)> {
    using return_type = R;
    using args_tuple  = std::tuple<Args...>;

    template<std::size_t I>
    using arg = std::tuple_element_t<I, args_tuple>;

    using class_type = void;

    static constexpr std::size_t arity = sizeof...(Args);

    using functional = std::function<R(Args...)>;
  };

  template<typename R, typename... Args>
  struct function_traits<R(*)(Args...)>
    : function_traits<R(Args...)> {};


  template<typename R, typename... Args>
  struct function_traits<R(&)(Args...)>
    : function_traits<R(Args...)> {};

  template<typename C, typename R, typename... Args>
  struct function_traits<R(C::*)(Args...)>
    : function_traits<R(Args...)> {
      using class_type = C;
  };

  template<typename C, typename R, typename... Args>
  struct function_traits<R(C::*)(Args...) const>
    : function_traits<R(Args...)> {
      using class_type = C;
  };

  template<typename T>
  struct function_traits
    : function_traits<decltype(&T::operator())> {};

  template<typename F>
  constexpr auto wrap(F&& f)
    -> typename function_traits<std::decay_t<F>>::functional
  {
    return typename function_traits<std::decay_t<F>>::functional(
      std::forward<F>(f)
    );
  }
} // namespace leasy::ul2
