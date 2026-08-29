//
// Created by @wys on 26/08/2026.
//
#pragma once

#define LEASY_CPP_SUPPORT_OR_CREATE(STD, CODE) \
  namespace STD {                              \
    CODE                                        \
  }

#define std_pre98_INT 1
#define std98_INT     199711L
#define std11_INT     201103L
#define std14_INT     201402L
#define std17_INT     201703L
#define std20_INT     202002L
#define std23_INT     202302L

#define LEASY_MKSTD(Y) \
  inline constexpr auto Y = Y##_INT; \
  namespace _##Y {}

#define HAS_STANDARD(REQUIRED) (__cplusplus >= REQUIRED##_INT)

#define INCLUDE_COMPATIBILITY_LAYER(STD) \
  using namespace leasy::stl2::compat::_##STD;

#define SUPPORT_IMPL_STD(std) leasy::stl2::compat::_##std

namespace leasy::stl2::compat {
  LEASY_MKSTD(std98);
  LEASY_MKSTD(std11);
  LEASY_MKSTD(std14);
  LEASY_MKSTD(std17);
  LEASY_MKSTD(std20);
  LEASY_MKSTD(std23);
} // namespace leasy::compat

namespace leasy::stl2 {
  using namespace std;

#if !HAS_STANDARD(std98)
  INCLUDE_COMPATIBILITY_LAYER(std98);
#endif

#if !HAS_STANDARD(std11)
  INCLUDE_COMPATIBILITY_LAYER(std11);
#endif

#if !HAS_STANDARD(std14)
  INCLUDE_COMPATIBILITY_LAYER(std14);
#endif

#if !HAS_STANDARD(std17)
  INCLUDE_COMPATIBILITY_LAYER(std17);
#endif

#if !HAS_STANDARD(std20)
  INCLUDE_COMPATIBILITY_LAYER(std20);
#endif

#if !HAS_STANDARD(std23)
  INCLUDE_COMPATIBILITY_LAYER(std23);
#endif

}
