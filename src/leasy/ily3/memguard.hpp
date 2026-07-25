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

#include <cstdlib>

namespace leasy::ily3 {
  template <typename T> struct guard final {
    T resource;
    void (*deleter)(T*) = [](T*) -> void {};

    inline guard() {}
    inline guard(const T &RES)
      : resource(RES) {}
    
    inline guard(const T &RES, void(*_deleter)(T*)) 
      : resource(RES), deleter(_deleter) {}
    
    inline ~guard() {
      deleter(&this->resource);
    }

    inline void operator=(const T &other) {
      deleter(&this->resource);
      this->resource = other;
    }

    inline void reset(void) {
      deleter(&this->resource);
      this->resource = T();
    }
  };
}