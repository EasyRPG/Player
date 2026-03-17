#pragma once

#include <vector>

#include "types.hpp"

namespace leasy::app {
  using onready_func = named_data<void(*)(void)>;
  void onready(const named_data<void(*)(void)> &func);

  using onprocess_func = named_data<void(*)(double)>;
}