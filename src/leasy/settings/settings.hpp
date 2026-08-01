#pragma once

#include <string>
#include "../ul2/state.hpp"
#include "../ily3/ily3.hpp"

namespace leasy::settings {
  template <typename T>
  inline T get(const std::string &name) {
    return ily3::global::state.get<T>("leasy.User.Settings." + name);
  }

  template <typename T>
  inline void set(const std::string &name, const T &v) {
    ily3::global::state.set<T>("leasy.User.Settings." + name, v);
  }
}