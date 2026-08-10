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
    // "... wrapped_type_name() [T = foo::bar]"
    constexpr std::string_view prefix = "T = ";
    constexpr std::string_view suffix = "]";
#elif defined(__GNUC__)
    // "... wrapped_type_name() [with T = foo::bar; std::string_view = ...]"
    constexpr std::string_view prefix = "with T = ";
    constexpr std::string_view suffix = ";";
#elif defined(_MSC_VER)
    // "... wrapped_type_name<foo::bar>(void)"
    constexpr std::string_view prefix = "wrapped_type_name<";
    constexpr std::string_view suffix = ">(void)";
#endif

    const auto start = wrapped.find(prefix);
    if (start == std::string_view::npos)
      return {};

    const auto type_begin = start + prefix.size();
    const auto type_end = wrapped.find(suffix, type_begin);

    return wrapped.substr(type_begin, type_end - type_begin);
  }

  extern void _leasy_cache_nameof_this(const std::type_index &, const std::string &);


  template<typename T>
  constexpr std::string_view canonical_primitive_name() {
    if constexpr (std::is_same_v<T, uint8_t>) {
      return "uint8_t";
    } else if constexpr (std::is_same_v<T, int8_t>) {
      return "int8_t";
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      return "uint16_t";
    } else if constexpr (std::is_same_v<T, int16_t>) {
      return "int16_t";
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      return "uint32_t";
    } else if constexpr (std::is_same_v<T, int32_t>) {
      return "int32_t";
    } else if constexpr (std::is_same_v<T, uint64_t>) {
      return "uint64_t";
    } else if constexpr (std::is_same_v<T, int64_t>) {
      return "int64_t";
    } else if constexpr (std::is_same_v<T, long double>) {
      return "ldouble";
    } else if constexpr (std::is_same_v<T, bool>) {
      return "bool";
    } else if constexpr (std::is_same_v<T, char>) {
      return "char";
    } else if constexpr (std::is_same_v<T, signed char>) {
      return "schar";
    } else if constexpr (std::is_same_v<T, unsigned char>) {
      return "uchar";
    } else if constexpr (std::is_same_v<T, short>) {
      return "short";
    } else if constexpr (std::is_same_v<T, unsigned short>) {
      return "ushort";
    } else if constexpr (std::is_same_v<T, int>) {
      return "int";
    } else if constexpr (std::is_same_v<T, unsigned int>) {
      return "uint";
    } else if constexpr (std::is_same_v<T, long>) {
      return "long";
    } else if constexpr (std::is_same_v<T, unsigned long>) {
      return "ulong";
    } else if constexpr (std::is_same_v<T, long long>) {
      return "llong";
    } else if constexpr (std::is_same_v<T, unsigned long long>) {
      return "ullong";
    } else if constexpr (std::is_same_v<T, float>) {
      return "float";
    } else if constexpr (std::is_same_v<T, double>) {
      return "double";
    } else {
      return {};
    }
  }

  template<typename T>
  constexpr std::string_view fake_nameof() {
    if constexpr (std::is_same_v<T, uint8_t>) return "uint8_t";
    else if constexpr (std::is_same_v<T, int8_t>) return "int8_t";
    else if constexpr (std::is_same_v<T, uint16_t>) return "uint16_t";
    else if constexpr (std::is_same_v<T, int16_t>) return "int16_t";
    else if constexpr (std::is_same_v<T, uint32_t>) return "uint32_t";
    else if constexpr (std::is_same_v<T, int32_t>) return "int32_t";
    else if constexpr (std::is_same_v<T, uint64_t>) return "uint64_t";
    else if constexpr (std::is_same_v<T, int64_t>) return "int64_t";
    else if constexpr (std::is_same_v<T, long double>) return "ldouble";
    else {
      constexpr auto name = detail::parse_type_name(detail::wrapped_type_name<T>());
      return name;
    }
  }
} // namespace detail

template<typename T>
constexpr std::string_view nameof() {
  auto fake = detail::parse_type_name(detail::wrapped_type_name<T>());
  detail::_leasy_cache_nameof_this(typeid(T), std::string(fake));
  return fake;
}

extern std::string nameof(const std::type_index &idx);
