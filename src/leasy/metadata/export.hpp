//
// Created by Hüseyin ÖZTÜRK on 03/08/2026.
//

#pragma  once

#include "type.hpp"

#include <vector>

namespace leasy::metadata {
  template<typename T>
  inline std::shared_ptr<Class> make_pointer() {
    return make_class<T*>()
          .method("value", [](const T *p){ return *p; })
          .done();
  }

  template<typename T>
  inline std::shared_ptr<Class> make_vector_data() {
    return make_class<std::vector<T> >()
        .method("size",
                [](const std::vector<T> &v) {
                  return v.size();
                })
        .method("empty",
                [](const std::vector<T> &v) {
                  return v.empty();
                })
        .method("clear",
                [](std::vector<T> &v) {
                  v.clear();
                })
        .method("pop_back",
                [](std::vector<T> &v) {
                  v.pop_back();
                })
        .method("front",
                [](std::vector<T> &v) -> T & {
                  return v.front();
                })
        .method("back",
                [](std::vector<T> &v) -> T & {
                  return v.back();
                })
        .method("at",
                [](std::vector<T> &v, size_t i) -> T & {
                  return v.at(i);
                })
        .method("reserve",
                [](std::vector<T> &v, size_t n) {
                  v.reserve(n);
                })
        .method("capacity",
                [](const std::vector<T> &v) {
                  return v.capacity();
                });

    if constexpr (std::is_copy_constructible_v<T>) {
      c.method("push_back",
               [](std::vector<T> &v, const T &x) {
                 v.push_back(x);
               });
    }

    if constexpr (std::is_default_constructible_v<T>) {
      c.method("resize",
               [](std::vector<T> &v, size_t n) {
                 v.resize(n);
               });
    }

    c.done();
  }
}
