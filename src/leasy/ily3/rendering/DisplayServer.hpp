#pragma once

#include "../basetypes.hpp"
#include <string>

namespace leasy::ily3 {
  class DisplayServer {
  public:
    twin<int> size;
    twin<int> pos;
  };
}