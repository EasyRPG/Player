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

#include <string>
#include <typeindex>
#include <unordered_map>

#include "nameof.hpp"

namespace detail {

  static std::unordered_map<std::type_index, std::string> &nameof_cache() {
    static std::unordered_map<std::type_index, std::string> c = {};
    return c;
  }

  void _leasy_cache_nameof_this(const std::type_index &idx, const std::string &name) {
    auto &cache = nameof_cache();
    if (cache.find(idx) == cache.end()) {
      cache[idx] = name;
    }
  }

}

std::string nameof(const std::type_index &idx) {
  auto cache = detail::nameof_cache();
  if (cache.find(idx) != cache.end()) {
    return cache[idx];
  }

  return idx.name();
}
