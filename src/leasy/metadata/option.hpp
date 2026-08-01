#pragma once

#include <cstdlib>

namespace leasy::metadata {
  struct  options {
    size_t  indent      = 2;
    bool    newlines    = true;
    bool    space_colon = true;
    bool    space_equal = true;
  };
}