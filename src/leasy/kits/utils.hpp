#pragma once

#include <type_traits>

namespace leasy::kits {
  template <typename T>
  using ptr_t = std::remove_reference_t<std::remove_cv_t<T>>;
}