#include <string>
#include <fstream>
#include <iostream>
#include <functional>
#include <unordered_map>

#include "../lio.hpp"

#include "../kits/strings.hpp"
#include "../kits/ps7k.hpp"

#include "../cli/cli.hpp"

#include "json.hpp"
#include "lua.hpp"
#include "namespace.hpp"


namespace leasy::metadata {
  std::unordered_map<std::string, std::function<void(std::ostream &, const Object&, const options&)>> map = {
    {"lua", lua::write },
    {"json", json::write },
  };

  void Xdump(const std::string &lang, const options &options) {
    if (map.find(lang) != map.end()) {
      std::ofstream of("dump." + lang);
      map[lang](of, EasyRPG().dump(), options);
    } else {
      io().Warning.writeln(__func__, ": language ", lang, " not found!");
    }
  }
}

using namespace leasy;

namespace {
  std::vector<std::string> languages;
  metadata::options           options = {};

  auto ok = []() {
    cli::addcli("--Xdump-lang=[L]", [&](cli::CliArgs args) {
      languages.push_back(args["L"]);
    });

    cli::addcli("--Xdump-ident=[N]", [&](cli::CliArgs args) {
      options.indent = kits::from_string_or_default<size_t>(args["N"]);
    });

    cli::addcli("--Xdump-newlines=[B]", [&](cli::CliArgs args) {
      options.newlines = kits::lower(args["B"]) == "yes";
    });

    cli::addcli("--Xdump", [&](auto) {
      for (const auto&lang: languages) {
        io().System.writeln("Xdump: dumping for ", lang, "...");
        metadata::Xdump(lang, options);
      }
    });

    return false;
  }();
}