#pragma once

#include <memory>
#include "baseui.h"
#include "game_config.h"

namespace leasy::ui3 {
  std::shared_ptr<BaseUi> make_win(int w, int h, int xpos = 120, int ypos = 120, int maxfps = 60);

  void update_windows();
}