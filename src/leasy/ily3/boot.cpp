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
    auto lua_file = p / "leasy.user.lua";

    if (!fs::exists(lua_file)) {
      io.Error.writeln(lua_file, ": file not found! (cannot load any mod!)");
      return;
    }
		global::state.dostring(lscripts::set_require);
    global::state.dofile(lua_file.string());
  }
}
