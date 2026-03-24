#include "version.h"

#include "../ldebug.hpp"
#include "../ul2/state.hpp"

namespace leasy::ily3 {
  namespace global {
    ul2::lstate state = ([](void) -> ul2::lstate {
      LDBG("creating instance");
      return ul2::lstate(true);
    })();
  }

  namespace lua_scripts {
    const char *init = 
R"(leasy.user = {}
leasy.user.ready = function() end
leasy.user.process = function() end
leasy.user.draw = function() end)";
  }

  void setup_lua(void) { 
    global::state.push("leasy.system.version.full", Version::GetVersionString());
    global::state.push("leasy.system.version.major", Version::MAJOR);
    global::state.push("leasy.system.version.minor", Version::MINOR);
    global::state.push("leasy.system.version.fix", Version::PATCH);
    global::state.dostring(lua_scripts::init);
  }
}