#pragma once

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
}