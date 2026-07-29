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
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/

#pragma once

#include <any>
#include <vector>
#include <typeindex>
#include <functional>
#include "../ul2/function_traits.hpp"
#include "../kits/rtt.hpp"

namespace leasy::iky7 {
  template <typename Tuple, typename F, std::size_t... Is>
  std::any invoke_from_any_impl(const F& f, const std::vector<std::any>& args, std::index_sequence<Is...>) {
    using tuple = std::remove_reference_t<Tuple>;
    using traits = ul2::function_traits<F>;

    auto eval = kits::is_callable_with(kits::tuple_types<typename traits::args_tuple>(), args);
    if (! eval.first) {
      throw std::runtime_error(eval.second);
    }

    if constexpr (std::is_void_v<std::invoke_result_t<F, std::tuple_element_t<Is, tuple>...>>) {
      std::invoke((f),std::any_cast<std::tuple_element_t<Is, tuple>>(args[Is])...);
      return {};
    } else {
      return std::any{
        std::invoke((f), std::any_cast<std::tuple_element_t<Is, tuple>>(args[Is])...)
      };
    }
  }

  template <typename Fn>
  auto bridgefunc(const Fn& f) {
    using Callable = std::decay_t<Fn>;
    using traits = ul2::function_traits<Callable>;
    using args_tuple = typename traits::args_tuple;

    return [f = Callable(f)](const std::vector<std::any>& args) -> std::any {
      auto eval = kits::is_callable_with(kits::tuple_types<args_tuple>(), args);
 
      if (!eval.first) throw std::runtime_error(eval.second);
 
      return invoke_from_any_impl<args_tuple>(
        f,
        args,
        std::make_index_sequence<std::tuple_size_v<args_tuple>>{}
      );
    };
  }
}