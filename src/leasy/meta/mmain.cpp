#include <memory>

#include "main.hpp"
#include "scene.h"
#include "scene_logo.h"
#include "metascene.hpp"
#include "../lio.hpp"
#include "../iky7/nameof.hpp"

namespace leasy::meta2 {
  bool is_meta2_enabled = false;

  void setmeta2(bool v) {
    is_meta2_enabled = v;
    if (v) {
      io().System.writeln("using meta2");
    }
  }
}