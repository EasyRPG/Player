#pragma once

#include "color.h"

namespace leasy::ily3 {
  template <typename TX, typename TY> struct pair {
    TX x;
    TY y;
  };

  template <typename T> struct twin {
    T x, y;

    inline twin() {}
    inline twin(const T &a, const T &b)
      : x(a), y(b) {}
  };

  /* dummy function! */
  template <typename T> twin<T> make_twin(const T &a, const T &b) {
    return twin<T>(a, b);
  }

  template <typename TX, typename TY> pair<TX, TY> make_pair(const TX &x, const TY &y) {
    return pair<TX, TY> {
      .x = x, .y = y
    };
  }

  inline Color color_from_u32(uint32_t v) {
    Color col;
    col.red = (v >> 24)   & 0xFF;
    col.green = (v >> 16) & 0xFF;
    col.blue = (v >> 8)   & 0xFF;
    col.alpha = (v)       & 0xFF;
    return col;
  }
}