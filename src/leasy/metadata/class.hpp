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
#include <vector>
#include <typeindex>
#include <type_traits>

#include "../iky7/nameof.hpp"
#include "../iky7/anyf.hpp"
#include "../kits/rtt.hpp"
#include "../kits/strings.hpp"
#include "../ul2/state.hpp"
#include "../ul2/function_traits.hpp"

#include "function.hpp"
#include "type.hpp"
#include "metadata.hpp"

namespace leasy::metadata {

  struct NativeClass : public Data {
    std::string                               name = "";
    std::unordered_map<std::string, NativeCallable>   methods = {};
    TypeInfo                                  type;

    inline NativeClass() {}
    inline NativeClass(
      const std::string &_name, 
      const std::unordered_map<std::string, NativeCallable> &_methods = {},
      const TypeInfo &idx = {}
    ) : name(_name), methods(_methods), type(idx) {}

    template <typename... Fs>
    inline NativeClass &method(const std::string &name, Fs&&... fn) {

      this->methods[name] = make_function(name, std::forward<Fs>(fn)...);

      return *this;
    }

    inline Object dump() const override {
      Map t;
      t.add("name", this->name)
       .add("TypeInfo", this->type.dump());
      
      Map methods;
      for (const auto &method: this->methods) {
        methods[method.first] = method.second.dump();
      }
      t.add("methods", methods);
      return t;
    }

    inline void bind(ul2::lstate &state) const override {
      auto name = kits::replace(this->name, "::", ".");

      for (const auto &method: this->methods) {
        state.bind2(name + "." + method.first, method.second.lua());
      }
    }
  };

  template <typename T>
  NativeClass make_class() {
    return NativeClass(
      std::string(nameof<T>()), {}, typeidof<T>()
    );
  }
}