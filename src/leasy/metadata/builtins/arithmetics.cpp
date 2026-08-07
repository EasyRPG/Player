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

//
// Created by @wys on 06/08/2026.
//

#include <memory>
#include <string>
#include <charconv>
#include <type_traits>

#include "../type.hpp"
#include "../Assembly.hpp"
#include "leasy/metadata/Domain.hpp"

namespace {
  template<typename T>
  T parse_numeric(const std::string &s) {
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
    } else {
      return T{};
    }
  }

  template<>
  bool parse_numeric<bool>(const std::string &s) {
    if (s == "true" || s == "1") return true;
    return false;
  }

  template<typename U>
  struct numeric_type {
    using T = std::remove_reference_t<std::remove_cv_t<U> >;
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
  void make_types(leasy::metadata::BuiltInAssembly &a) {
    (a.addType<Ts>(numeric_type<Ts>::build()), ...);
  }
}


namespace leasy::metadata {
  namespace {
    const auto OK = []() {
      BuiltInAssembly assembly("leasy::std");

      make_types<
        uint8_t, uint16_t, uint32_t, uint64_t,
        int8_t, int16_t, int32_t, int64_t,
        bool,
        float, double, long double
      >(assembly);

      AppDomain().load(std::make_shared<BuiltInAssembly>(assembly));
      return false;
    }();
  }
}
