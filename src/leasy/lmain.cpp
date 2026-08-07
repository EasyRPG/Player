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


#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "ldebug.hpp"
#include "lio.hpp"
#include "ily3/ily3.hpp"

#include "iky7/time.hpp"
#include "cli/cli.hpp"

namespace leasy {
  namespace settings {
    void makedefault();
  }

  namespace app {
    void ready(void);

    leasy::ios::attachment logfile = io().Debug.attach(ios::file("leasy.io.Debug.log"));
    
    void lmain(const std::vector<std::string> &args) {
      try {
        io().System.writeln(__func__, ": leasy subsystem started!");

        ily3::setup();
        ily3::setup_lua();
        settings::makedefault(); // These will get overriden by the next load!
        ily3::boot(std::filesystem::current_path());
        // TODO: add the fs::current_path() to glob
        cli::cli(args);
        ready();
      } catch (const std::exception &e) {
        io().Error.writeln("exception during lmain() init!!");
        io().Error.writeln(e.what());
        io().Warning.writeln(__func__, ": execution may continue, but this can crash because of init-exceptions!");
      }
    }
  }
}
