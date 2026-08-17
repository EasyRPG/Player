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
#include <unordered_map>

#include "bitmap.h"

#define MAX_NODE_STR_MACRO(X) #X

namespace leasy::meta2::node {
  static inline const char *assemblyName = "leasy::meta2::node";

  class Meta2Context;
  extern Meta2Context meta2Context;

  class Node {
  public:
    using node_id = uint64_t;
    static constexpr node_id INVALID_NODE = std::numeric_limits<node_id>::max();
    static constexpr node_id MAX_NODE = std::numeric_limits<node_id>::max() - 1;

    static node_id nextId;
    static std::stack<node_id>& freeIds();

    static node_id getNewId() {
      if (freeIds().empty()) {
        if (nextId >= MAX_NODE) {
          throw std::runtime_error("too much nodes! (no free ID remaining)");
        }
        return nextId++;
      }

      const auto I{freeIds().top()};
      freeIds().pop();
      return I;
    }

  private:
    friend Meta2Context;

    std::unordered_map<node_id, std::shared_ptr<Node>> _children;
    node_id id{INVALID_NODE};

    bool removeRequested{false};

    template <typename Callback>
    void iterate(const Callback &callback) {
      std::stack<node_id> cleanupList{};

      for (auto &[ref, node]: _children) {
        if (node && !node->removeRequested) {
          callback(node);
        } else {
          cleanupList.push(ref);
        }
      }

      while (! cleanupList.empty()) {
        _children.erase(cleanupList.top());
        cleanupList.pop();
      }
    }

    void _ready() {
      this->ready();

      iterate([](const std::shared_ptr<Node>& node) {
        node->_ready();
      });
    }

    void _update(double v) {
      update(v);

      iterate([&v](const std::shared_ptr<Node>& node) {
        node->_update(v);
      });
    }

    void _draw(Bitmap *map) {
      draw(map);

      iterate([&map](const std::shared_ptr<Node>& node) {
        node->_draw(map);
      });
    }

  public:
    virtual ~Node() {
      freeIds().push(this->id);
    }

    Node() {
      this->id = getNewId();
    }

    virtual void ready() {}
    virtual void update(double) {}
    virtual void draw(Bitmap*) {}

    void remove() {
      removeRequested = true;
    }

    auto getChildren() const { return _children; }

    void addChild(const std::shared_ptr<Node> &nodeptr) {
      _children[nodeptr->id] = nodeptr;
    }

    bool isValid() const {
      return this->id != INVALID_NODE;
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

  inline Node::node_id Node::nextId {0};
  inline std::stack<Node::node_id> &Node::freeIds() {
    static auto hold = new std::stack<Node::node_id>();
    return *hold;
  };
}