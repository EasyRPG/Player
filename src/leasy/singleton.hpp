#pragma once

namespace leasy {
  class Singleton {
  public:
    template <typename T>
    static inline T &get_singleton() {
      static T instance = T();
      return instance;
    }

    template <typename T>
    static inline T &set_singleton(const T &V) {
      get_singleton<T>() = V;
      return get_singleton<T>();
    }
  };
}