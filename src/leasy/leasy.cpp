
#include "bitmap.h"
#include "sprite.h"
#include "text.h"
#include "baseui.h"
#include "drawable_mgr.h"
#include "image_png.h"
#include "filesystem_native.h"

#include <fstream>
#include <string_view>
#include "ldebug.hpp"
#include "ul2/state.hpp"
#include "ily3/ily3.hpp"
#include "libs2/drawing/image.hpp"

namespace leasy {
  namespace app {
    bool should_exit = false;
    std::unique_ptr<libs2::Image> image;

    bool exit_requested() {
      return should_exit;
    }
    
    void request_exit() {
      should_exit = true;
    }

    void ready(void) {
      ily3::global::state.call<void>("leasy.user.ready");
      image = std::make_unique<libs2::Image>("/Users/wys/Documents/easy/icon.png", libs2::Image::Type::PNG);
    }
    
    void process(void) {
      ily3::global::state.call<void>("leasy.user.process");
    }

    void draw(Bitmap *map) {
      ily3::global::state.call<void>("leasy.user.draw");
      image->Draw(*map);
      image->Position().x += 1; 
      image->Position().y += 1; 
    }

    void exit(void) { }
  }
}