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

#pragma once

#include <memory>
#include <vector>
#include "../../metadata/namespace.hpp"
#include "../../iky7/cursor.hpp"

namespace leasy::meta2::node {
  class Node {
  private:
    std::vector<std::shared_ptr<Node>> _children;

  public:
    inline virtual void ready() {}
    inline virtual void update(double) {}
    inline virtual void draw() {}
    inline virtual ~Node() = default;
    inline Node() = default;

    inline std::vector<std::shared_ptr<Node>> children() { return _children; }

    inline auto visit() {
      return iky7::make_cursor(this->_children);
    }
  };
}