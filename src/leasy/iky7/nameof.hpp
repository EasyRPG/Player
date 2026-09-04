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

#include <string>
#include <typeindex>
#include <string_view>

#include "leasy/lio.hpp"
#include "leasy/kits/utils.hpp"
#include "leasy/types/string.hpp"

namespace detail {
  template<typename T>
  constexpr std::string_view wrapped_type_name() {
#if defined(__clang__) || defined(__GNUC__)
    return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    return __FUNCSIG__;
#else
#   error Unsupported compiler
#endif
  }

  constexpr std::string_view parse_type_name(std::string_view wrapped) {
#if defined(__clang__)
    constexpr std::string_view prefix = "T = ";
    constexpr std::string_view suffix = "]";
#elif defined(__GNUC__)
    constexpr std::string_view prefix = "with T = ";
    constexpr std::string_view suffix = ";";
#elif defined(_MSC_VER)
    constexpr std::string_view prefix = "wrapped_type_name<";
    constexpr std::string_view suffix = ">(";
#endif

    const auto start = wrapped.find(prefix);
    if (start == std::string_view::npos)
      return {};

    const auto type_begin = start + prefix.size();
    const auto type_end = wrapped.find(suffix, type_begin);

    if (type_end == std::string_view::npos)
      return {};

    return wrapped.substr(type_begin, type_end - type_begin);
  }

  extern void _leasy_cache_nameof_this(const std::type_index &, const std::string &);

} // namespace detail

template<typename T>
leasy::String nameof() {
  auto fake = leasy::String(detail::parse_type_name(detail::wrapped_type_name<T>()));
  if (fake.endsWith("]")) fake = fake.substr(0, fake.size() - 1);

  detail::_leasy_cache_nameof_this(typeid(T), std::string(fake.data(), fake.size()));
  return fake;
}

extern std::string nameof(const std::type_index &idx);
