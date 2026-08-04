//
// Created by Hüseyin ÖZTÜRK on 02/08/2026.
//

#ifndef EASYRPG_PLAYER_SPRITE2D_HPP
#define EASYRPG_PLAYER_SPRITE2D_HPP

#include "node2d.hpp"
#include "sprite.h"
#include <filesystem>

namespace leasy::meta2::node {
  class Sprite2D : public metadata::Enable<Sprite2D, Node2D> {
  protected:
  public:
    Sprite2D();
    Sprite2D(const std::filesystem::path&);
    void start() override;
    void draw() override;
  };
}

#endif //EASYRPG_PLAYER_SPRITE2D_HPP
