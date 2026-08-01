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


#include <string>
#include <utility>
#include <typeinfo>
#include <typeindex>
#include <string_view>
#include <unordered_map>

#include <bit>
#include <cmath>
#include <charconv>
#include <type_traits>

#include "../lio.hpp"
#include "type.hpp"
#include "namespace.hpp"

#include "color.h"

namespace detail {
  
  static std::unordered_map<std::type_index, std::string> &nameof_cache() {
    static std::unordered_map<std::type_index, std::string> c = {};
    return c;
  }
  
  void _leasy_cache_nameof_this(const std::type_index &idx, const std::string &name) {
    auto &cache = nameof_cache();
    if (cache.find(idx) == cache.end()) {
      cache[idx] = name;
    }
  }
}
namespace leasy::STATIC_LOCAL {
  void leasy_register_builtins_please_please(metadata::NSpace&);
}

namespace leasy::metadata {

  std::unordered_map<std::type_index, std::shared_ptr<Class>> &registery() {
    static std::unordered_map<std::type_index, std::shared_ptr<Class>> m = {};
    return m;
  }

  static std::pair<bool, std::string> trygetname(const std::type_index &idx) {
    auto& cache = detail::nameof_cache();
    if (cache.find(idx) != cache.end()) {
      return {true, cache[idx]};
    }
    return {false, idx.name()};
  }

  void _make_type(const std::type_index &idx, const std::shared_ptr<Class> &cls) {
    // FIXME: add overwrite options?
    registery()[idx] = cls;
  }

  std::shared_ptr<Class> typeidof(const std::type_index &index) {
    auto &reg = registery();

    if (reg.find(index) == reg.end()) {
      auto r = trygetname(index);
      io().Warning.writeln(__func__, ": type ", index.name(), " (", r.second, ") does not have any loadable metadata ...");
      reg[index] = std::make_shared<UnresolvedClass>(UnresolvedClass(index, r.first ? r.second : index.name()));
    }

    return reg[index];
  }

  bool is_same_type(const std::type_index &cindex, const std::shared_ptr<Class> &classptr) {
    return classptr->cindex() == cindex;
  }

  NSpace &EasyRPG() {
    static NSpace ns = []() -> NSpace {
      NSpace n = NSpace("EasyRPGPlayer");
      STATIC_LOCAL::leasy_register_builtins_please_please(n);
      return n;
    }();
    return ns;
  }
}

namespace leasy::STATIC_LOCAL {
  template <typename T>
  inline T parse_numeric(const std::string& s) {
    if constexpr (std::is_integral_v<T>) {
      T value{};
      auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);

      if (ec != std::errc{} || ptr != s.data() + s.size())
        throw std::invalid_argument("Invalid number");

      return value;
    } else if constexpr (std::is_same_v<T, float>) {
      return std::stof(s);
    } else if constexpr (std::is_same_v<T, double>) {
      return std::stod(s);
    } else if constexpr (std::is_same_v<T, long double>) {
      return std::stold(s);
    }
  }

  template <>
  inline bool parse_numeric<bool>(const std::string& s) {
    if (s == "true" || s == "1") return true;
    return false;
  }

  template <typename T>
  struct numeric_type {
    static_assert(std::is_arithmetic_v<T>);

    static std::shared_ptr<leasy::metadata::Class> build() {
      auto c = leasy::metadata::make_class<T>();

      c.method("new", 
        [](T v) -> T { return T{v}; },
        [](std::string s) { return parse_numeric<T>(s); },
        []() -> T { return T{}; }
      );

      c.method("add", [](T a, T b) { return a + b; });
      c.method("sub", [](T a, T b) { return a - b; });
      c.method("mul", [](T a, T b) { return a * b; });

      if constexpr (!std::is_same_v<T, bool>) {
        c.method("div", [](T a, T b) { return a / b; });
        c.method("is_signed", [](T) { return std::is_signed_v<T>; });
      } else {
        c.method("div", [](T a, T b) { return T{}; }); // Yeah.
        c.method("is_signed", [](T) { return false; }); // SORRY.
      }

      c.method("eq", [](T a, T b) { return a == b; });
      c.method("ne", [](T a, T b) { return a != b; });
      c.method("lt", [](T a, T b) { return a < b; });
      c.method("le", [](T a, T b) { return a <= b; });
      c.method("gt", [](T a, T b) { return a > b; });
      c.method("ge", [](T a, T b) { return a >= b; });
      c.method("is_zero", [](T x) { return x == T{}; });

      if constexpr (std::is_signed_v<T>) {
        c.method("is_positive", [](T x) { return x > 0; });
        c.method("is_negative", [](T x) { return x < 0; });
        c.method("abs", [](T x) { return std::abs(x); });
      } else {
        c.method("is_positive", [](T x) { return false; });
        c.method("is_negative", [](T x) { return false; });
        c.method("abs", [](T x) { return T{}; });
      }

      if constexpr (std::is_integral_v<T>) {
        c.method("is_even", [](T x) { return (x & 1) == 0; });

        c.method("is_odd", [](T x) { return (x & 1) != 0; });

        if constexpr (!std::is_same_v<T, bool>) {
          c.method("mod", [](T a, T b) { return a % b; });
        }
      } else {
        c.method("floor", [](T x) { return std::floor(x); });
        c.method("ceil", [](T x) { return std::ceil(x); });
        c.method("round", [](T x) { return std::round(x); });
        c.method("sqrt", [](T x) { return std::sqrt(x); });
        c.method("sin", [](T x) { return std::sin(x); });
        c.method("cos", [](T x) { return std::cos(x); });
        c.method("tan", [](T x) { return std::tan(x); });
        c.method("is_nan", [](T x) { return std::isnan(x); });
        c.method("is_inf", [](T x) { return std::isinf(x); });
        c.method("is_finite", [](T x) { return std::isfinite(x); });
        c.method("epsilon", [] { return std::numeric_limits<T>::epsilon(); });
      }

      c.method("min", [] { return std::numeric_limits<T>::lowest(); });
      c.method("max", [] { return std::numeric_limits<T>::max(); });

      return c.done();
    }
  };

  template <typename... Ts>
  void add_numeric_types(leasy::metadata::NSpace& sp) {
    (sp.add(numeric_type<Ts>::build()), ...);
  }

  void leasy_register_builtins_please_please(leasy::metadata::NSpace &sp) {
    add_numeric_types<
      char, int8_t, int16_t, int32_t, int64_t,
      uint8_t, uint16_t, uint32_t, uint64_t,
      bool, float, double, long double
    >(sp);

    sp.add(leasy::metadata::make_class<Color>()
        .method("new", 
          [](uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) { return Color(red, green, blue, alpha);  },
          []() { return Color(); }
        )
        .done()
      );

  }
}