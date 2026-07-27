#pragma once

#include <unordered_map>
#include <string>

#include "metadata.hpp"
#include "function.hpp"
#include "class.hpp"

namespace leasy::metadata {

  struct NSpace;
  inline NSpace make_namespace(const std::string&);

  struct NSpace : public Data {
    std::string name;

    std::unordered_map<std::string, NativeCallable> functions = {};
    std::unordered_map<std::string, NativeClass> classes = {};
    std::unordered_map<std::string, NSpace> namespaces = {};

    inline NSpace() {}

    inline NSpace(const std::string &_name)
      : name(_name) {}

    template <typename... Fs>
    inline NSpace &function(const std::string &name, Fs&&... fn) {
      this->functions[name] = make_function(name, std::forward<Fs>(fn)...);

      return *this;
    }

    inline NSpace &add(const NativeClass &klass) {

      this->classes[klass.name] = klass;

      return *this;
    }

    inline NSpace &add(const NSpace &ns) {

      this->namespaces[ns.name] = ns;

      return *this;
    }

    inline NSpace &sub(const std::string &name) {
      
      if (this->namespaces.find(name) == this->namespaces.end()) {
        this->namespaces[name] = make_namespace(name);
      }

      return this->namespaces[name];
    }

    inline Object dump() const override {
      Map t;

      t.add("name", this->name);

      Map functions;
      for (const auto &fn: this->functions) {
        functions[fn.first] = fn.second.dump();
      }

      Map classes;
      for (const auto &klass: this->classes) {
        classes[klass.first] = klass.second.dump();
      }

      Map namespaces;
      for (const auto &ns: this->namespaces) {
        namespaces[ns.first] = ns.second.dump();
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
          fn.second.lua()
        );
      }

      for (const auto &klass: this->classes) {

        auto &cl = klass.second;

        for (const auto &method: cl.methods) {
          state.bind2(
            prefix + "." + cl.name + "." + method.first,
            method.second.lua()
          );
        }
      }

      for (const auto &ns: this->namespaces) {
        ns.second.bind(state);
      }
    }
  };


  inline NSpace make_namespace(const std::string &name) {
    return NSpace(name);
  }

  NSpace &EasyRPG();
}

