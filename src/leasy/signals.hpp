#pragma once

#include <functional>
#include <vector>

#include "kits/preload.hpp"

namespace leasy {
  template<typename... Args>
  class Signal {
  public:
    using Callback = std::function<void(Args...)>;

    kits::preload_result connect(Callback cb) {
      callbacks.push_back([cb = std::move(cb)](Args... args) {
        std::puts("wtf?");
        cb(args...);
      });
      return {};
    }

    void emit(Args... args) {
      for (auto& cb : callbacks) {
        cb(args...);
      }
    }

  private:
    std::vector<Callback> callbacks;
  };
}