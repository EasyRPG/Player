#pragma once

#include <memory>

#include "scene.h"
#include "scene_logo.h"
#include "metascene.hpp"
#include "../lio.hpp"
#include "../iky7/nameof.hpp"

namespace leasy::meta2 {
  extern bool is_meta2_enabled;

  extern void setmeta2(bool);

  inline std::shared_ptr<Scene> make_main_scene() {
    if (is_meta2_enabled) {
      return std::make_shared<Scene_Meta>();
    } else {
      return std::make_shared<Scene_Logo>();
    }
  }
}