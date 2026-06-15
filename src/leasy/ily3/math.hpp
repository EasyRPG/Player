#pragma once

#include "basetypes.hpp"

namespace leasy::ily3 {
  template <typename T>
  inline T clamp(const T &min, const T &max, const T &v) {
    if (v < min) return min;
    else if (v > max) return max;
    return v;
  }
}