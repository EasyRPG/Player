//
// Created by @wys on 27/08/2026.
//

#pragma once

#include <functional>
#include <utility>
#include "leasy/kits/support.hpp"

namespace SUPPORT_IMPL_STD(std20) {
  template<class F, class A>
  struct _bind_obj {
    F originalFunc;
    A arg;

    template<class... Args>
    inline auto operator()(Args &&... a) {
      return std::invoke(originalFunc, arg, std::forward<Args>(a)...);
    }

    _bind_obj(F &&_originalFunc, A &&_arg) : originalFunc(std::forward<F>(_originalFunc)),
                                             arg(std::forward<A>(_arg)) {
    }
  };

  template<class F, class A>
  auto bind_front(F &&func, A &&arg) {
    return _bind_obj<F, A>(
      std::forward<F>(func),
      std::forward<A>(arg)
    );
  }

  template<class F, class FirstA, class... A>
  auto bind_front(F &&func, FirstA &&firstA, A &&... a) {
    return bind_front(
      bind_front(
        std::forward<F>(func),
        std::forward<FirstA>(firstA)
      ),
      std::forward<A>(a)...
    );
  }
}

namespace leasy::stl2 {
  template <typename>
  struct pass_t;

  template <typename C, typename R, typename... Args>
  struct pass_t<R(C::*)(Args...)> {
    using ArgsT = std::tuple<C, Args...>;

    static auto pass(R(C::*fn)(Args...)) {
      return std::function<R(C&, Args...)>(
        [fn](C &self, Args... args) {
          return std::invoke(fn, self, std::forward<Args>(args)...);
        }
      );
    }
  };

  template <typename C, typename R, typename... Args>
  struct pass_t<R(C::*)(Args...) const> {
    static auto pass(R(C::*fn)(Args...) const) {
      return std::function<R(const C&, Args...)>(
        [fn](const C& self, Args... args) {
          return std::invoke(fn, self, std::forward<Args>(args)...);
        }
      );
    }
  };

  template <auto F>
  auto pass() {
    return pass_t<decltype(F)>::pass(F);
  }
}