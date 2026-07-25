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


#include <filesystem>
#include <iostream>
#include <string>

#include "ldebug.hpp"
#include "lio.hpp"
#include "ily3/ily3.hpp"

#include "iky7/time.hpp"

namespace leasy {
  namespace app {
    void ready(void);

    leasy::ios::attachment logfile = io.Debug.attach(ios::file("leasy.io.Debug.log"));
    
    void lmain() {
      io.System.writeln(__func__, ": leasy subsystem started!");
      ily3::setup();
      ily3::setup_lua();
      ily3::boot(std::filesystem::current_path());
			// TODO: add the fs::current_path() to glob
      ready();
    }
  }
}
