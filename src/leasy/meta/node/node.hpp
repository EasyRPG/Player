#pragma once

#include <memory>

namespace leasy::meta2::node {
  class Node {
  protected:
  public:
    inline virtual void ready() {}
    inline virtual void update(double) {}
    inline virtual void draw() {}
  };
}