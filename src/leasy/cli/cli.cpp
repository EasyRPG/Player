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

#include <sstream>
#include <string>
#include <unordered_map>

#include "cli.hpp"
#include "../lio.hpp"
#include "../kits/cppsupport/bases.hpp"

namespace leasy::cli {

  CliParseResult parse_args(const std::string &pattern,
                            const std::string &input) {
    CliParseResult result;

    std::size_t pp = 0;
    std::size_t ip = 0;

    auto fail = [&](std::string message) -> CliParseResult {
      result.success = false;
      result.error = std::move(message);
      result.args.clear();
      return result;
    };

    while (pp < pattern.size()) {
      // Match literal character
      if (pattern[pp] != '[') {
        if (ip >= input.size())
          return fail("Unexpected end of input.");

        if (pattern[pp] != input[ip]) {
          return fail("Expected '" + std::string(1, pattern[pp]) + "', found '" +
                      std::string(1, input[ip]) + "'.");
        }

        ++pp;
        ++ip;
        continue;
      }

      // Parse variable name
      auto end = pattern.find(']', pp);
      if (end == std::string::npos)
        return fail("Malformed pattern: missing closing ']'.");

      std::string key = pattern.substr(pp + 1, end - pp - 1);
      pp = end + 1;

      // Determine following literal
      auto nextVar = pattern.find('[', pp);
      std::string literal = pattern.substr(
          pp, nextVar == std::string::npos ? pattern.size() - pp : nextVar - pp);

      // Variable at end of pattern
      if (literal.empty()) {
        result.args[key] = input.substr(ip);
        ip = input.size();
        break;
      }

      auto pos = input.find(literal, ip);
      if (pos == std::string::npos) {
        return fail("Expected literal \"" + literal + "\" after argument '" +
                    key + "'.");
      }

      result.args[key] = input.substr(ip, pos - ip);

      ip = pos + literal.size();
      pp += literal.size();
    }

    if (ip != input.size())
      return fail("Unexpected trailing characters.");

    result.success = true;
    return result;
  }

  std::string make_indicator(std::size_t begin, std::size_t length = 1) {
    if (length == 0)
      length = 1;

    return std::string(begin, ' ') + std::string(length, '^');
  }

  std::string format_parse_error(const CliParseResult &result,
                                const std::string &pattern,
                                const std::string &input) {
    if (result.success)
      return {};

    std::ostringstream ss;

    ss << "Parse error!";

    if (!result.error.empty())
      ss << ": " << result.error;

    ss << "\n\n";

    ss << "Pattern\n";
    ss << "  " << pattern << '\n';
    ss << "  " << make_indicator(result.pattern_pos) << '\n';

    ss << "\n";

    ss << "Input\n";
    ss << "  " << input << '\n';
    ss << "  " << make_indicator(result.input_pos);

    return ss.str();
  }

  CliMap &reg() {
    static CliMap m = {};
    return m;
  }

  void addcli(const std::string &pattern, const CliFunc &func) {
    reg()[pattern] = func;
  }

  std::vector<std::string> remove_easyrpg_player_args(const std::vector<std::string>&);

  void cli(const std::vector<std::string> &args) {
    size_t argp = 0;
    for (const auto&arg: remove_easyrpg_player_args(args)) {
      bool matched = false;

      for (const auto&[pattern, func]: reg()) {
        auto result = parse_args(pattern, arg);
        if (result.success) {
          func(result.args);
          matched = true;
          break;
        }
      }

      if (! matched) io().Error.writeln("error: unknown command: ", arg, " pos #", argp);
      argp++;
    }
  }

  std::vector<std::string> easyrpgArgs = {
    "--autobattle-algo",
    "-c", "--config-path", "--encoding",
    "--enemyai-algo", "--engine", "--font1",
    "--font1-size", "--font2", "--font2-size",
    "--font-path", "--language", "--load-game-id",
    "--log-file", "--new-game", "--no-log-color",
    "--no-rtp", "--patch-antilag-switch", "--patch-common-this",
    "--patch-direct-menu", "--patch-dynrpg", "--patch-easyrpg",
    "--patch-key-patch", "--patch-maniac", "--patch-pic-unlock",
    "--patch-rpg2k3-cmds", "--no-patch", "--project-path",
    "--record-input", "--replay-input", "--rtp-path",
    "--save-path", "--seed", "--fps-limit",
    "--no-fps-limit", "--fullscreen", "--game-resolution",
    "--pause-focus-lost", "--no-pause-focus-lost", "--scaling",
    "--show-fps", "--fps-render-window", "--no-show-fps",
    "--stretch", "--no-stretch", "--vsync",
    "--no-vsync", "--window", "--no-audio",
    "--music-volume", "--sound-volume", "--soundfont",
    "--soundfont-path", "--battle-test", "--hide-title",
    "--start-map-id", "--start-party", "--start-position",
    "--test-play", "-v", "--version",
    "-h", "--help", "BattleTest",
    "HideTitle", "TestPlay", "Window"
  };

  std::vector<std::string> remove_easyrpg_player_args(const std::vector<std::string> &args) {
    std::vector<std::string> r;
    r.reserve(args.size());

    for (size_t i = 1; i < args.size(); i++) {
      if (! stl2::compat::cpp20::contains(easyrpgArgs, args[i])) r.push_back(args[i]);
    }

    return r;
  }
} // namespace leasy::cli

namespace {
  auto H = [] {
    leasy::cli::addcli("-cancel", [](auto) {
      leasy::io().System.writeln("Cancellation was requested by the CLI-interface, shutting down...");
      leasy::io().Debug.writeln("Bye-bye (this one in DEBUG handle so devs can see haha)");
      exit(0);
    });

    return char{};
  }();
}
