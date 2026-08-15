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

#include "bitmap.h"
#include "../../iky7/cursor.hpp"

namespace leasy::meta2::node {
  static inline const char *assemblyName = "leasy::meta2::node";

  class Meta2Context;

  class Node {
    friend Meta2Context;
    std::vector<std::shared_ptr<Node>> _children;

    void _ready() {
      this->ready();

      for (const auto &child: _children) {
        child->ready();
      }
    }

    void _update(double v) {
      update(v);

      for (const auto &child: _children) {
        child->update(v);
      }
    }

    void _draw(Bitmap *map) {
      draw(map);

      for (const auto &child: _children) {
        child->draw(map);
      }
    }

  public:
    virtual ~Node() = default;
    Node() = default;
    virtual void ready() {}
    virtual void update(double) {}
    virtual void draw(Bitmap*) {}

    std::vector<std::shared_ptr<Node>> getChildren() const { return _children; }

    auto visit() {
      return iky7::make_cursor(this->_children);
    }

    void addChild(const std::shared_ptr<Node> &nodeptr) {
      this->_children.push_back(nodeptr);
      nodeptr->_ready();
    }
  };

  class Meta2Context {
  protected:
    std::shared_ptr<Node> root;

  public:
    Meta2Context(const std::shared_ptr<Node> &r): root{r} {}

    void ready() const {
      root->_ready();
    }

    void update(double d) const {
      root->_update(d);
    }

    void draw(Bitmap*m) const {
      root->_draw(m);
    }

    std::shared_ptr<Node> getRoot() const {
      return root;
    }
  };
}