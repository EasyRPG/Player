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

#include <string>
#include <vector>
#include <filesystem>

#include "../lua/lua.hpp"
#include "../ul2/dispatcher.hpp"
#include "../ul2/state.hpp"
#include "../ldebug.hpp"
#include "../lio.hpp"
#include "_lua_scripts.hpp"

namespace fs = std::filesystem;

namespace leasy::ily3 {

  namespace global {
    extern ul2::lstate state;
  }

  void boot(const fs::path &p) {
    try {
      auto lua_file = p / "leasy.user.lua";
      auto settings_file = p / "leasy.settings.lua";

      global::state.dostring(lscripts::set_require);

      if (!fs::exists(settings_file)) {
        io().Warning.writeln(settings_file, ": file not found! (default settings will be loaded!)");
      } else {
        global::state.dofile(settings_file.string());
      }

      if (!fs::exists(lua_file)) {
        io().Error.writeln(lua_file, ": file not found! (cannot load any mod!)");
      } else {
        global::state.dofile(lua_file.string());
      }
    } catch (const std::exception &e) {
      io().Error.writeln(__func__, ": c++ exception caught!");
      io().Error.writeln(__func__, ": ", e.what());
    }
  }
}
