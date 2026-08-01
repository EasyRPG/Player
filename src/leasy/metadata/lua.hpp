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

#include <ostream>
#include <sstream>
#include <string>
#include <cstddef>
#include <cstdio>
#include "object.hpp"

#include "option.hpp"

namespace leasy::metadata::lua {
  void  write(std::ostream&, const Object&, const options& = {});
  std::string  dump(const Object&, const options& = {});

  namespace detail {
    void  pad(std::ostream&, const options&, size_t depth);
    void  escape(std::ostream&, const string&);
    void  emit(std::ostream&, const Object&, const options&, size_t depth);
  }

  inline void detail::pad(std::ostream &os, const options &opts, size_t depth) {
    if (!opts.newlines) return;
    os << '\n';
    for (size_t i = 0; i < depth * opts.indent; ++i) os << ' ';
  }

  inline void detail::escape(std::ostream &os, const string &s) {
    os << '"';
    for (unsigned char c : s) {
      switch (c) {
        case '"':  os << "\\\""; break;
        case '\\': os << "\\\\"; break;
        case '\n': os << "\\n";  break;
        case '\t': os << "\\t";  break;
        case '\r': os << "\\r";  break;
        default:
          if (c < 0x20) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\%03d", c);
            os << buf;
          } else {
            os << static_cast<char>(c);
          }
      }
    }
    os << '"';
  }

  // Table keys are always emitted as ["key"] rather than bare identifiers.
  // Slightly more verbose, but sidesteps Lua's reserved-word and
  // valid-identifier rules entirely.
  inline void detail::emit(std::ostream &os, const Object &obj, const options &opts, size_t depth) {
    const char *equal = opts.space_equal ? " = " : "=";

    obj
      .when<null>([&](const null&) { os << "nil"; })
      .when<boolean>([&](const boolean &v) { os << (v ? "true" : "false"); })
      .when<integer>([&](const integer &v) { os << v; })
      .when<uinteger>([&](const uinteger &v) { os << v; })
      .when<number>([&](const number &v) { os << v; })
      .when<string>([&](const string &v) { escape(os, v); })
      .when<Array>([&](const Array &v) {
        auto items = v.values();
        os << '{';
        for (size_t i = 0; i < items.size(); ++i) {
          if (i) os << ',';
          pad(os, opts, depth + 1);
          emit(os, items[i], opts, depth + 1);
        }
        if (!items.empty()) pad(os, opts, depth);
        os << '}';
      })
      .when<Map>([&](const Map &v) {
        auto items = v.values();
        size_t i = 0;
        os << '{';
        for (const auto &[k, val] : items) {
          if (i++) os << ',';
          pad(os, opts, depth + 1);
          os << '[';
          escape(os, k);
          os << ']' << equal;
          emit(os, val, opts, depth + 1);
        }
        if (!items.empty()) pad(os, opts, depth);
        os << '}';
      });
  }

  inline void write(std::ostream &os, const Object &obj, const options &opts) {
    os << "return ";
    detail::emit(os, obj, opts, 0);
  }

  inline std::string dump(const Object &obj, const options &opts) {
    std::ostringstream oss;
    write(oss, obj, opts);
    return oss.str();
  }
}
