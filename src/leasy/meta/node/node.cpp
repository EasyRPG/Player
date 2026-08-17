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

#include "node.hpp"
#include "leasy/metadata/Domain.hpp"

namespace leasy::meta2::node {
  namespace {
    auto reg = []() {
      auto Asm = metadata::AppDomain().getAssemblyOrCreate<metadata::BuiltInAssembly>(assemblyName);

      Asm->addType<Node>(metadata::make_class<Node>()
        .method("ready", [](Node &self) { self.ready(); })
        .method("update", [](Node &self, double delta) { self.update(delta); })
        .method("draw", [](Node &self, Bitmap* ref) { self.draw(ref); })
        .method("new" ,[]() { return Node(); })
        .method("children", [](const Node &self) { return self.getChildren(); })
        .method("addChild", [](Node &self, const std::shared_ptr<Node> &child) {
          return self.addChild(child);
        })
        .done()
      );

      Asm->addFunction("addChildToMain", [&](const std::shared_ptr<Node> n) {
        meta2Context.getRoot()->addChild(n);
      });

      return false;
    }();
  }
}