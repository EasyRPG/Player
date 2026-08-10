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

#include <utility>

namespace leasy::kits {

  template<typename Range, typename F>
  auto select(const Range& range, F&& func) {
    using E = typename Range::value_type;
    using R = decltype(func(std::declval<E>()));

    std::vector<R> result;

    for (const auto& e: range) {
      result.push_back(func(e));
    }

    return result;
  }

  template<typename Range, typename Pred>
  auto where(const Range& range, Pred&& pred) {
    using E = typename Range::value_type;

    std::vector<E> result;

    for (const auto& e : range) {
      if (pred(e)) {
        result.push_back(e);
      }
    }

    return result;
  }

  template<typename Range, typename Pred>
  bool any(const Range& range, Pred&& pred) {
    for (const auto& e : range) {
      if (pred(e)) {
        return true;
      }
    }

    return false;
  }

  template<typename Range, typename Pred>
  bool all(const Range& range, Pred&& pred) {
    for (const auto& e : range) {
      if (!pred(e)) {
        return false;
      }
    }

    return true;
  }

  template<typename Range, typename Pred>
  auto first(const Range& range, Pred&& pred)
    -> const typename Range::value_type* {
    for (const auto& e : range) {
      if (pred(e)) {
        return &e;
      }
    }

    return nullptr;
  }

  template<typename Range, typename Pred>
  std::size_t count(const Range& range, Pred&& pred) {
    std::size_t result = 0;

    for (const auto& e : range) {
      if (pred(e)) {
        ++result;
      }
    }

    return result;
  }

  template<typename Range>
  auto sum(const Range& range) {
    using T = typename Range::value_type;

    T result{};

    for (const auto& e : range) {
      result += e;
    }

    return result;
  }

  template<typename Range, typename F>
  auto sum(const Range& range, F&& func) {
    using E = typename Range::value_type;
    using R = decltype(func(std::declval<E>()));

    R result{};

    for (const auto& e : range) {
      result += func(e);
    }

    return result;
  }

  template <typename Range, typename F>
  auto foreach(const Range &range, F&& func) {
    using E = typename Range::value_type;
    using R = decltype(func(std::declval<E>()));

    if constexpr (! std::is_void_v<R>) {
      std::vector<R> vec;

      for (const auto &e: range) {
        vec.emplace_back(func(e));
      }

      return vec;
    } else {
      for (const auto &e: range) {
        func(e);
      }
    }
  }
}