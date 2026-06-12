#include <filesystem>
#include <string>

#include "../../ily3/libsys.hpp"
#include "../../ily3/basetypes.hpp"
#include "../../ily3/drawqueue.hpp"
#include "../../lio.hpp"

using namespace leasy;
namespace fs = std::filesystem;

namespace {
  static std::string fullpath(const std::string &p) {
    try {
      return fs::absolute(p);
    } catch (...) {
      return p;
    }
  }
  
  static bool mkdir(const std::string &p) {
    return fs::create_directories(p);
  }

  static std::uintmax_t rmdir(const std::string &p) {
    try {
      return fs::remove_all(p);
    } catch (...) { return 0; }
  }

  bool autoboot = ([]() -> bool {
    ily3::addlib(ily3::lib {
      .name = "fs",
      .funcs = { ily3::make_fun<fullpath>("fullpath") }
    });
    
    return true;
  })();
}