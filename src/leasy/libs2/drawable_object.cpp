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
 *  0xEF9087A
 * 
 * **********************************************************************/
#include "drawable.h"
#include "image_png.h"
#include "drawable_object.hpp"
#include "../ldebug.hpp"
#include "../ily3/memguard.hpp"
#include "../ily3/basetypes.hpp"

namespace leasy::libs2 {
  
  Opacity *DrawableObject::opacity() {
    return &this->opa;
  }

  ily3::twin<int> *DrawableObject::position() {
    return &this->pos;
  }

  ily3::twin<int> *DrawableObject::size() {
    return &this->siz;
  }

  DrawableObject::DrawableObject() : Drawable(255) {}

  Rect DrawableObject::get_rect() {
    return Rect(pos.y, pos.x, siz.x, siz.y);
  }
}