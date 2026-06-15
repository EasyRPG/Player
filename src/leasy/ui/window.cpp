#include "window.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include "baseui.h"
#include "game_config.h"

namespace leasy::ui3 {
  std::unordered_map<unsigned long long, std::weak_ptr<BaseUi>> windows;
  unsigned long long id = 0;

  std::shared_ptr<BaseUi> make_win(int w, int h, int xpos, int ypos, int maxfps) {
    Game_Config cfg;
    cfg.video.window_height.Set(h);
    cfg.video.window_width.Set(w);
    cfg.video.fps_limit.Set(maxfps);
    cfg.video.window_x.Set(xpos);
    cfg.video.window_y.Set(ypos);
    cfg.video.fullscreen.Set(false);
    auto win = BaseUi::CreateUi(w, h, cfg);
    
    win->SetPauseWhenFocusLost(false);
    windows[id++] = win;
    
    return win;
  }

  void update_windows() {
    // As we may not modify an unordered map during its iteration, we gon' free 'em later.
    std::vector<unsigned long long> tofree;

    for (const auto&[i, win]: windows) {
      if (win.expired()) {
        tofree.push_back(i);
      } else {
        win.lock()->UpdateDisplay();
      }
    }

    for (const auto&id: tofree) {
      windows.erase(id);
    }
  }
}
