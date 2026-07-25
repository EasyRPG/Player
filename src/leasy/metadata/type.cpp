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


#include <string>
#include <utility>
#include <typeinfo>
#include <typeindex>
#include <string_view>
#include <unordered_map>

#include "../lio.hpp"
#include "type.hpp"
#include "namespace.hpp"

namespace leasy::metadata {
  std::unordered_map<std::type_index, TypeInfo> &registery() {
    static std::unordered_map<std::type_index, TypeInfo> m = {};
    return m;
  }

  TypeInfo make_metadata(const std::type_index &info) {
    if (registery().find(info) == registery().end()) {
      io.Warning.writeln("missing type info ", info.name(), ", hash ", info.hash_code());
      return TypeInfo(info);
    } else {
      return registery()[info];
    }
  }

  void _register_metadata_type(const std::type_info &info, const TypeInfo &metadata) {
    registery()[info] = metadata;
  }

  NSpace &EasyRPG() {
    static NSpace ns("EasyRPGPlayer");
    return ns;
  }
}