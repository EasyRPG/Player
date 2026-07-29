#pragma once

#include "scene_logo.h"
#include "bitmap.h"

namespace leasy::meta2 {
  class Scene_Meta : public Scene_Logo {
  public:
    inline void Start() override {

    }

    inline void vUpdate() override {

    }

    void DrawBackground(Bitmap&) override {

    }
  };
}