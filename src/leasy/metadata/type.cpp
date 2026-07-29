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

namespace leasy::metadata {
  std::unordered_map<std::type_index, std::shared_ptr<Class>> &registery() {
    static std::unordered_map<std::type_index, std::shared_ptr<Class>> m = {};
    return m;
  }

  static std::pair<bool, std::string> trygetname(const std::type_index &idx) {
    auto& cache = detail::nameof_cache();
    if (cache.find(idx) != cache.end()) {
      return {true, cache[idx]};
    }
    return {false, idx.name()};
  }

  void _make_type(const std::type_index &idx, const std::shared_ptr<Class> &cls) {
    // FIXME: add overwrite options?
    registery()[idx] = cls;
  }

  std::shared_ptr<Class> typeidof(const std::type_index &index) {
    auto &reg = registery();

    if (reg.find(index) == reg.end()) {
      auto r = trygetname(index);
      io().Warning.writeln(__func__, ": type ", index.name(), " (", r.second, ") does not have any loadable metadata ...");
      reg[index] = std::make_shared<UnresolvedClass>(UnresolvedClass(index, r.first ? r.second : index.name()));
    }

    return reg[index];
  }

  bool is_same_type(const std::type_index &cindex, const std::shared_ptr<Class> &classptr) {
    return classptr->cindex() == cindex;
  }

  NSpace &EasyRPG() {
    static NSpace ns("EasyRPGPlayer");
    return ns;
  }
}

