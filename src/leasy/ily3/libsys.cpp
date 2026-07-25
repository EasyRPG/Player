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
 *  0xEF9087A
 * 
 * **********************************************************************/
#include "libsys.hpp"

#include <string>
#include <vector>
#include "../lua/lua.hpp"
#include "../ul2/dispatcher.hpp"
#include "../ul2/state.hpp"
#include "../ldebug.hpp"

namespace leasy::ily3 {
  namespace global {
    extern ul2::lstate state;
  }

  std::vector<lib> libs;

  void addlib(const lib &l) {
    libs.push_back(l);
  }

  void setup(void) {
    for (const auto &l: libs) {
      for (const auto &fun: l.funcs) {
        global::state.push(l.name + "." + fun.name, fun.fn);
      }
    }

    // then destroy the vector cuz why not ?
    libs = {};
  }
}