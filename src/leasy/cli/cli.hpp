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

#pragma once

#include <string>
#include <functional>
#include <unordered_map>

namespace leasy::cli {
  using CliArgs = std::unordered_map<std::string, std::string>;
  using CliFunc = std::function<void(CliArgs)>;
  using CliMap  = std::unordered_map<std::string, CliFunc>;

  struct CliParseResult {
    bool success = false;
    CliArgs args;
    std::string error;
    std::size_t pattern_pos = 0;
    std::size_t input_pos = 0;
  };

  void addcli(const std::string&, const CliFunc&);
  void cli(const std::vector<std::string>&);
}