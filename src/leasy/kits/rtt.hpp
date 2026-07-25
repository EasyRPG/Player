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

#include <any>
#include <string>
#include <vector>
#include <typeindex>

#include "../metadata/type.hpp"

namespace leasy::kits {
  inline std::vector<metadata::TypeInfo> extract_types(const std::vector<std::any> &objects) {
    std::vector<metadata::TypeInfo> indexes;
    indexes.reserve(objects.size());
    
    for (const auto& object: objects) {
      indexes.push_back(metadata::TypeInfo(object.type()));
    }
    
    return indexes;
  }

  inline std::pair<bool, std::string> is_callable_with(const std::vector<metadata::TypeInfo> &types, const std::vector<std::any> &args) {
    if (types.size() != args.size()) return {false, "argument size mismatch! expected " + std::to_string(types.size()) + " got " + std::to_string(args.size())};
    else {
      for (size_t i = 0; i < types.size(); i++) {
        if (types[i].index() != args[i].type()) return {
          false, "argument #" + std::to_string(i) + " mismatch!"
        };
      }
    }
    
    return {true, "success"};
  }
  
//  template<typename... Ts>
//  std::vector<metadata::TypeInfo> tuple_types() {
//    return { metadata::TypeInfo::from<Ts>()... };
//  }

  template<typename Tuple, std::size_t... Is>
  std::vector<metadata::TypeInfo>
  tuple_types_impl(std::index_sequence<Is...>)
  {
    return {
        metadata::TypeInfo::from<std::tuple_element_t<Is, Tuple>>()...
    };
  }

  template<typename Tuple>
  std::vector<metadata::TypeInfo> tuple_types()
  {
    return tuple_types_impl<Tuple>(
        std::make_index_sequence<std::tuple_size_v<Tuple>>{}
    );
  }
}