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

#include "node2d.hpp"
#include "leasy/metadata/Domain.hpp"

namespace leasy::meta2::node {
  namespace {
    auto reg = []() {
      auto Asm = metadata::AppDomain().getAssemblyOrCreate<metadata::BuiltInAssembly>(assemblyName);
      auto cls = metadata::make_class<Node2D>()
          .method("new" ,
                  []() { return Node2D(); },
                  [](pos_t x, pos_t y) { return Node2D(x, y); }
          )
          .method("pos", [](Node2D &self) { return self.pos().tuple(); })
          .method("move", [](Node2D &self, pos_t x, pos_t y) { return self.move(x, y); })
          .done();

      Asm->addType<Node2D>(cls);
      return false;
    }();
  }
}