//
// Created by @wys on 09/08/2026.
//

#include "lua_glues.hpp"
#include "leasy/cli/cli.hpp"

namespace leasy::metadata::glues {
  namespace {
    bool okay = [] {
      cli::addcli("--Xglue-gen=[L]", [](auto args) {
        if (kits::lower(args["L"]) == "lua") {
          std::filesystem::create_directories("assembly");
          generateLuaGlue(luaExportAssemblyPrefixNiceNice, AppDomain(), "assembly");
        }
      });

      return false;
    }();
  }
}
