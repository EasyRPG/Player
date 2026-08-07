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
// Created by @wys on 06/08/2026.
//

#include "leasy/metadata/Domain.hpp"

namespace leasy::metadata {
  namespace {
    const auto A = []() {
      const auto& assembly = AppDomain().getAssemblyOrCreate<BuiltInAssembly>("leasy::std");
      assembly->addType<std::string>(make_class<std::string>().done());
      assembly->addType<std::string_view>(
        make_class<std::string_view>()
        .method("get", [](const std::string_view &view) { return std::string(view); })
        .done()
      );

      assembly->addFunction("nameof", make_function("nameof", static_cast<std::string(*)(const std::type_index &)>(&nameof)));

      return false;
    }();
  }
}
