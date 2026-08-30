//
// Created by @wys on 30/08/2026.
//

#pragma once

#include <optional>

#include "function.hpp"

namespace leasy::metadata {
  template<typename T, typename Member>
  constexpr auto readonly(Member T::*member) {
    return [member](const T &object) {
      return object.*member;
    };
  }

  template <typename T, typename Member>
  constexpr auto field(Member T::*member) {
    return [member](T& self, std::optional<Member> value) -> Member {
      if (value) {
        self.*member = *value;
      }

      return self.*member;
    };
  }
}
