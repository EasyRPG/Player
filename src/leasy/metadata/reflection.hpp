#pragma once

#include <vector>
#include <functional>
#include "function.hpp"
#include "class.hpp"
#include "namespace.hpp"

#include "../kits/preload.hpp"

namespace leasy::metadata {
  extern std::vector<std::function<void(void)>> preload_list;

  template <typename F>
  kits::preload_result load(const F &f) {
    preload_list.push_back(f);
    return {};
  }
}