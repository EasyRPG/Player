#pragma once

#include <tuple>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

#include "lua_stack.hpp"

namespace leasy::ul2 {
  template <typename T, typename = void> struct lua_class;

  template <typename T, auto ... Methods>
  struct lua_class<T, std::enable_if_t<std::is_class_v<T>>> {
    
  };

}
