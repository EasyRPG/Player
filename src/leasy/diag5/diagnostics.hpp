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

#include "../lio.hpp"
#include "here.h"

#include <string>

namespace leasy::diag5 {
  struct dgpoint {
    std::string _name;
    std::string _id;

    template <typename ...Args>
    inline dgpoint(const std::string &name, Args&&... args)
      : _name(name) {
      io.Debug.writeln("<dg> diagnostic point created <", name, ">");
      (io.Debug.write(args, " "), ...);
    }

    inline ~dgpoint() {
      io.Debug.writeln("<dg> diagnostic point destroyed <", _name, ">");
    }
  };
}

#define dumpvout(X) leasy::io.Debug.writeln(here, " <dump> @", #X, ": ", X)
#define dumpvasp(X) #X, X