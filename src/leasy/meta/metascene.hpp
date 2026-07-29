#pragma once

#include "scene_logo.h"
#include "bitmap.h"

namespace leasy::meta2 {
  class Scene_Meta : public Scene_Logo {
  public:
  };

  class Scene_Test : public Scene {
  public:
    inline Scene_Test() {
      throw;
    }
  };
}