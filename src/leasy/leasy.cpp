#include <fstream>
#include <string_view>
#include "events.hpp"
#include "ldebug.hpp"
#include "ul2/state.hpp"
#include "ily3/ily3.hpp"
#include "libs2/image/image.hpp"
#include "diag5/here.h"

namespace leasy {
  namespace ily3 {
    extern std::vector<std::shared_ptr<Drawable>> leasy_draw_queue;
  }

  namespace app {
    bool should_exit = false;

    bool exit_requested() {
      return should_exit;
    }

    void request_exit() {
      should_exit = true;
    }

    void ready(void) {
      // Use a chrono + ... so i can bench start-time ?
      // Still ain't done

      ily3::global::state.call<void>("leasy.User.ready");
    }
    
    void process(void) {
      ily3::global::state.call<void>("leasy.User.process");
    }

    void draw(Bitmap *map) {
      ily3::global::state.call<void>("leasy.User.draw");
      
      for (const auto &drawable: ily3::leasy_draw_queue) {
        drawable->Draw(*map);
      }
      
      std::vector<std::shared_ptr<Drawable>> queue;
      queue.reserve(32);
      ily3::leasy_draw_queue.swap(queue);
    }

    void exit(void) { }
  }
}
