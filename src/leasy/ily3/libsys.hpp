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
      : name(name), fn(ul2::dispatch<F>)
      {}

    inline fun(const std::string &name, lua_CFunction fun)
      : name(name), fn(fun) {}
  };

  template <auto F>
  inline fun make_fun(const std::string& name) {
    return fun{ name, ul2::dispatch<F> };
  }

  struct lib {
    std::string name;
    std::vector<fun> funcs;
  };

  void addlib(const lib&);
}

