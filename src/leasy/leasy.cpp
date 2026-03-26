#include <fstream>
#include <string_view>
#include "ldebug.hpp"
#include "ul2/state.hpp"
#include "ily3/ily3.hpp"
#include "libs2/drawing/image.hpp"

namespace leasy {
  namespace app {
    bool should_exit = false;

    bool exit_requested() {
      return should_exit;
    }
    
    void request_exit() {
      should_exit = true;
    }

    void ready(void) {
      ily3::global::state.call<void>("leasy.user.ready");
    }
    
    void process(void) {
      ily3::global::state.call<void>("leasy.user.process");
    }

    void draw(Bitmap *map) {
      ily3::global::state.call<void>("leasy.user.draw");
    }

    void exit(void) { }
  }
}
