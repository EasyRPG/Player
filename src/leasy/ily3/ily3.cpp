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
#include "version.h"

#include "../ldebug.hpp"
#include "../ul2/state.hpp"
#include "../lio.hpp"

namespace leasy::ily3 {
  namespace global {
    ul2::lstate state = ([](void) -> ul2::lstate {
      return ul2::lstate(true);
    })();
  }

  namespace lua_scripts {
    const char *init = // The only reason of this is maybe to make the code easier to read. That's kinda all.
R"(leasy = leasy or {}
leasy.User = {}
leasy.User.ready = function() end
leasy.User.process = function() end
leasy.User.draw = function() end
leasy.Engine = {})";
  }

  static int lprint(lua_State* L) {
    int nargs = lua_gettop(L);
    std::ostringstream oss;

    for (int i = 1; i <= nargs; ++i) {
      const char* str = lua_tostring(L, i);
      if (str) oss << (str);
      else oss << (luaL_typename(L, i));
      if (i < nargs) oss << '\t';
    }

    io.System.writeln(oss.str());

    return 0;
  }

  static int lwarn(lua_State* L) {
    int nargs = lua_gettop(L);
    std::ostringstream oss;

    for (int i = 1; i <= nargs; ++i) {
      const char* str = lua_tostring(L, i);
      if (str) oss << (str);
      else oss << (luaL_typename(L, i));
      if (i < nargs) oss << '\t';
    }

    io.Warning.writeln(oss.str());

    return 0;
  }

  void setup_lua(void) { 
    global::state.push("leasy.System.version.full", Version::GetVersionString());
    global::state.push("leasy.System.version.major", Version::MAJOR);
    global::state.push("leasy.System.version.minor", Version::MINOR);
    global::state.push("leasy.System.version.fix", Version::PATCH);
    global::state.dostring(lua_scripts::init);
    global::state.push("print", lprint);
    global::state.push("warn", lwarn);
  }
}