#pragma once

#include <vector>
#include <memory>

namespace leasy::iky7 {
  class mmchunk {
  private:
    template <typename T>
    std::vector<std::shared_ptr<T>> &gel() {
      static std::vector<std::shared_ptr<T>> vsptr;
      return vsptr;
    }

  public:
    template <typename T>
    inline std::shared_ptr<T> pin(const std::shared_ptr<T> &p) {
      gel<T>().push_back(p);
      return p;
    }
  };

  extern mmchunk chunk;
}