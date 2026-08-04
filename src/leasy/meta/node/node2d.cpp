//
// Created by Hüseyin ÖZTÜRK on 02/08/2026.
//

#include "node2d.hpp"
#include "leasy/metadata/namespace.hpp"

namespace leasy::meta2::node {
  namespace {
    auto reg = []() {
      metadata::EasyRPG()
      .sub("leasy")
      .sub("meta2")
      .sub("node")
      .add(metadata::make_class<Node2D>()
        .method("new" ,
          []() { return Node2D(); },
          [](pos_t x, pos_t y) { return Node2D(x, y); }
        )
        .method("pos", [](Node2D &self) { return self.pos().tuple(); })
        .method("move", [](Node2D &self, pos_t x, pos_t y) { return self.move(x, y); })
        .done()
      );
      return false;
    }();
  }
}