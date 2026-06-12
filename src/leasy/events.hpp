#pragma once

#include <vector>
#include <string>

#include "ily3/ily3.hpp"
#include "lio.hpp"

namespace leasy::engine {
  template <class...Args>
  inline void event(const std::string &name, Args...args) {
    std::string rname = "leasy.Engine." + name;
    try {
      ily3::global::state.call<void>(rname, args...);
    } catch (const ul2::ulexception2 &e) {
      io.Error.writeln("error during event invocation: ", e.whut());
      // Sometimes, it's just because the callback does not exist!
      // So let's just ensure it haha
      ily3::global::state.dostring(rname + " = " + rname + " or function()end");
    } catch (const std::exception &e) {
      io.Error.writeln("error during event invocation: ", e.what());
    }
  }
}