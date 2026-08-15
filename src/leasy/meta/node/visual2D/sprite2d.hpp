/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/

//
// Created by @wys on 02/08/2026.
//

#ifndef EASYRPG_PLAYER_SPRITE2D_HPP
#define EASYRPG_PLAYER_SPRITE2D_HPP

#include "node2d.hpp"
#include "bitmap.h"
#include <filesystem>

namespace leasy::meta2::node {

  class Sprite2D : public Node2D {
  protected:
    std::shared_ptr<Bitmap> selfMap;
    Opacity opacity{};

  public:
    Sprite2D();
    Sprite2D(const std::filesystem::path&, bool=true);
    Sprite2D(std::istream*, bool=true);

    void draw(Bitmap*) override;

    void load(std::streambuf*, bool);
  };
}

#endif //EASYRPG_PLAYER_SPRITE2D_HPP
