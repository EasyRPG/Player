#include "color.h"
#include "settings.hpp"

namespace leasy::settings {
  template <typename T>
  static void scene(const std::string &name, const T &v) {
    set<T>("Scene." + name, v);
  }

  void makedefault() {
    scene<bool>("DebugValidate", false);

    set<bool>("Game.HasBackgroundColorOverride", true);
  }
}