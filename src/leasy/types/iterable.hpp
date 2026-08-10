//
// Created by @wys on 09/08/2026.
//

#pragma once
#include <functional>

namespace leasy {
  template <typename I, typename V>
  class Iterable {
  public:
    virtual ~Iterable() = default;

    using index_type = I;
    using value_type = V;

    virtual void forEach(const std::function<void(const I&, const V&)>&) const = 0;
    virtual bool hasValue(const V&) const = 0;
    virtual bool hasIndex(const I&) const = 0;
    [[nodiscard]] virtual size_t size() const = 0;
  };
}
