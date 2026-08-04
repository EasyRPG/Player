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

#include <unordered_map>
#include <string>

#include "metadata.hpp"
#include "function.hpp"
#include "../kits/strings.hpp"

namespace leasy::metadata {

  struct NSpace;
  inline NSpace make_namespace(const std::string&);

  struct NSpace final : public Data {
    std::string name;

    std::unordered_map<std::string, std::shared_ptr<function_base_t>> functions = {};
    std::unordered_map<std::string, std::shared_ptr<Class>> classes = {};
    std::unordered_map<std::string, std::shared_ptr<NSpace>> namespaces = {};

    inline NSpace() {}

    inline NSpace(const std::string &_name)
      : name(_name) {}

    template <typename... Fs>
    inline NSpace &function(const std::string &name, Fs&&... fn) {
      this->functions[name] = make_function(name, std::forward<Fs>(fn)...);

      return *this;
    }

    inline NSpace &add(const std::shared_ptr<Class> &klass) {

      this->classes[klass->fullname()] = klass;

      return *this;
    }

    inline NSpace &add(const NSpace &ns) {
      this->namespaces[ns.name] = std::make_shared<NSpace>(ns);

      return *this;
    }

    inline NSpace &sub(const std::string &name) {
      
      if (this->namespaces.find(name) == this->namespaces.end()) {
        this->namespaces[name] = std::make_shared<NSpace>(name);
      }

      return (*this->namespaces[name]);
    }

    inline Object dump() const override {
      Map t;

      t.add("name", this->name);

      Map functions;
      for (const auto &fn: this->functions) {
        functions[fn.first] = fn.second->dump();
      }

      Map classes;
      for (const auto &klass: this->classes) {
        classes[klass.first] = klass.second->dump();
      }

      Map namespaces;
      for (const auto &ns: this->namespaces) {
        namespaces[ns.first] = ns.second->dump();
      }

      t.add("functions", functions)
       .add("classes", classes)
       .add("namespaces", namespaces);

      return t;
    }

    inline void bind(ul2::lstate &state) const override {

      auto prefix = kits::replace(this->name, "::", ".");

      for (const auto &fn: this->functions) {
        state.bind2(
          prefix + "." + fn.first,
          fn.second->lua()
        );
      }

      for (const auto&klass: this->classes) {
        auto n = kits::replace(klass.second->fullname(), "::", ".");
        for (const auto &method: klass.second->methods()) {
          state.bind2(this->name + "." + n + "." + method.first, method.second->lua());
        }
      }

      for (const auto &ns: this->namespaces) {
        ns.second->bind2(state, this->name);
      }
    }

    inline void bind2(ul2::lstate &state, const std::string &prefix) const {
      for (const auto&f: this->functions) {
        // We won't use the bind() method, as it'll register it as
        // <Cursor>.<Fn>, where <Cursor> is most of the time _G!
        state.bind2(prefix + "." + this->name + "." + f.first, f.second->lua());
      }

      for (const auto&klass: this->classes) {
        auto n = kits::replace(klass.second->fullname(), "::", ".");

        for (const auto &method: klass.second->methods()) {
          state.bind2(n + "." + method.first, method.second->lua());
        }
      }

      for (const auto &ns: this->namespaces) {
        ns.second->bind2(state, prefix + "." + this->name);
      }
    }
  };

  inline NSpace make_namespace(const std::string &name) {
    return (name);
  }

  NSpace &EasyRPG();
}

