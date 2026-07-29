#include <bit>
#include <cmath>
#include <type_traits>

#include "namespace.hpp"

namespace leasy::metadata {
  template <typename T>
  struct numeric_type {
    static_assert(std::is_arithmetic_v<T>);

    static std::shared_ptr<Class> build() {
      auto c = make_class<T>();

      c.method("add", [](T a, T b) { return a + b; });
      c.method("sub", [](T a, T b) { return a - b; });
      c.method("mul", [](T a, T b) { return a * b; });

      if constexpr (!std::is_same_v<T, bool>) {
        c.method("div", [](T a, T b) { return a / b; });
        c.method("is_signed", [](T) { return std::is_signed_v<T>; });
      }

      c.method("eq", [](T a, T b) { return a == b; });
      c.method("ne", [](T a, T b) { return a != b; });
      c.method("lt", [](T a, T b) { return a < b; });
      c.method("le", [](T a, T b) { return a <= b; });
      c.method("gt", [](T a, T b) { return a > b; });
      c.method("ge", [](T a, T b) { return a >= b; });
      c.method("is_zero", [](T x) { return x == T{}; });

      if constexpr (std::is_signed_v<T>) {
        c.method("is_positive",
          [](T x) { return x > 0; });

        c.method("is_negative",
          [](T x) { return x < 0; });

        c.method("abs",
          [](T x) { return std::abs(x); });
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

  static auto R = []() {
    EasyRPG().add(numeric_type<bool>::build())
             .add(numeric_type<char>::build())
             .add(numeric_type<short>::build())
             .add(numeric_type<int>::build())
             .add(numeric_type<long>::build())
             .add(numeric_type<long long>::build())
             .add(numeric_type<unsigned char>::build())
             .add(numeric_type<unsigned short>::build())
             .add(numeric_type<unsigned int>::build())
             .add(numeric_type<unsigned long>::build())
             .add(numeric_type<unsigned long long>::build())
             .add(numeric_type<float>::build())
             .add(numeric_type<double>::build())
             .add(numeric_type<long double>::build());
    return false;
  }();
}