#pragma once

#include <string>

namespace leasy {
  /**
   * @brief Represents a simple struct that holds a named element, in order to make 
   * debugging easier.
   */
  template <typename TData>
  struct named_data {
    std::string name;
    TData       data;
  };
}
