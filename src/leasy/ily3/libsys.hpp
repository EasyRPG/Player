#pragma once

#include <string>
#include <vector>
#include "../lua/lua.hpp"
#include "../ul2/dispatcher.hpp"

/** BY THE WAY.
 * ily3 stands for I Love You :3, don't you guys search for something complex lol
 */

namespace leasy::ily3 {
  struct fun {
    std::string name;
    lua_CFunction fn;

    inline fun() : name(".."), fn(nullptr) {} // So can't be pushed hahahaha
    
    template <auto F> inline fun(const std::string &name)
      : fn(ul2::dispatch<F>)
      {}
  };

  struct lib {
    std::string name;
    std::vector<fun> funcs;
  };

  void addlib(const lib&);
}