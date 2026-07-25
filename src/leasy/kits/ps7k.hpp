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

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <iterator>

#if __cplusplus < 202002L

namespace leasy::compat {

template <class Container, class T>
typename Container::size_type erase(Container& c, const T& value) {
  auto old = c.size();
  c.erase(std::remove(c.begin(), c.end(), value), c.end());
  return old - c.size();
}

template <class Container, class Pred>
typename Container::size_type erase_if(Container& c, Pred pred) {
  auto old = c.size();
  c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
  return old - c.size();
}

template <class Container, class T>
bool contains(const Container& c, const T& value) {
  return std::find(c.begin(), c.end(), value) != c.end();
}

template <class E>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

inline bool starts_with(std::string_view s, std::string_view prefix) {
  return s.substr(0, prefix.size()) == prefix;
}

inline bool ends_with(std::string_view s, std::string_view suffix) {
  return s.size() >= suffix.size() &&
    s.substr(s.size() - suffix.size()) == suffix;
}

template <class To, class From>
std::enable_if_t<
  sizeof(To) == sizeof(From) &&
  std::is_trivially_copyable_v<From> &&
  std::is_trivially_copyable_v<To>,
  To>
bit_cast(const From& src) noexcept {
  To dst;
  std::memcpy(&dst, &src, sizeof(To));
  return dst;
}

template <class T, class U>
constexpr bool cmp_equal(T t, U u) noexcept {
  if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
    return t == u;
  else if constexpr (std::is_signed_v<T>)
    return t >= 0 && std::make_unsigned_t<T>(t) == u;
  else
    return u >= 0 && t == std::make_unsigned_t<U>(u);
}

template <class T, class U>
constexpr bool cmp_less(T t, U u) noexcept {
  if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
    return t < u;
  else if constexpr (std::is_signed_v<T>)
    return t < 0 || std::make_unsigned_t<T>(t) < u;
  else
    return u >= 0 && t < std::make_unsigned_t<U>(u);
}

template <class T, class U>
constexpr bool cmp_greater(T t, U u) noexcept {
  return cmp_less(u, t);
}

template <class T, class U>
constexpr bool cmp_less_equal(T t, U u) noexcept {
  return !cmp_greater(t, u);
}

template <class T, class U>
constexpr bool cmp_greater_equal(T t, U u) noexcept {
  return !cmp_less(t, u);
}

} // namespace compat
#else
#warning leasy.System.Compiler: [WARNING] This mod was designed to support C++17 — C++20 may cause issues.
#endif