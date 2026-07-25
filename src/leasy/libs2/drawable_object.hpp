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

#pragma once

#include "drawable.h"
#include "image_png.h"
#include "../ldebug.hpp"
#include "../ily3/memguard.hpp"
#include "../ily3/basetypes.hpp"

namespace leasy::libs2 {
  class DrawableObject : public Drawable {
  protected:
    ily3::twin<int>         pos;
    ily3::twin<int>         siz;
    Opacity                 opa;

  public:

    Opacity *opacity();
    ily3::twin<int> *position();
    ily3::twin<int> *size();
    Rect get_rect();
    DrawableObject();
  };
}