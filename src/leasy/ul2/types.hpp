#pragma once

#include <type_traits>

namespace leasy::ul2 {
  template <typename T>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
}