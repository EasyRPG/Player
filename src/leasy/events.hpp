/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/

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
      io.Warning.writeln("error (ul2) during event invocation: ", e.whut());
      // Sometimes, it's just because the callback does not exist!
      // So let's just ensure it haha
      ily3::global::state.dostring(rname + " = " + rname + " or function()end");
    } catch (const std::exception &e) {
      io.Warning.writeln("error (stdc++) during event invocation: ", e.what());
    }
  }
}