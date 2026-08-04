//
// Created by Hüseyin ÖZTÜRK on 02/08/2026.
//

#include "node.hpp"
#include "leasy/metadata/namespace.hpp"

namespace leasy::meta2::node {
  namespace {
    auto reg = []() {
      metadata::EasyRPG()
      .sub("leasy")
      .sub("meta2")
      .sub("node")
      .add(metadata::make_class<Node>()
        .method("ready", [](Node &self) { self.ready(); })
        .method("update", [](Node &self, double delta) { self.update(delta); })
        .method("draw", [](Node &self) { self.draw(); })
        .method("new" ,[]() { return Node(); })
        .method("children", [](Node &self) { return self.children(); })
        .method("visit", [](Node &self) { return self.visit(); })
        .done()
      );
      return false;
    }();
  }
}