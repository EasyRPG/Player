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
// Created by @wys on 05/08/2026.
//

#ifndef EASYRPG_PLAYER_ASSEMBLY_HPP
#define EASYRPG_PLAYER_ASSEMBLY_HPP

#include <string>
#include <utility>
#include "type.hpp"
#include "function.hpp"
#include "../iky7/nameof.hpp"


namespace leasy::metadata {
  class Assembly : public Data, public SizeDescriptor {
  protected:
    std::string _name;
    std::string _luaDumpPrefix;

  public:
    inline Assembly() = default;

    inline Assembly(std::string name) : _name(std::move(name)) {
    }

    [[nodiscard]] inline virtual std::string name() const { return _name; }
    [[nodiscard]] inline virtual std::shared_ptr<Class> getType(const std::type_index &) const { return nullptr; }
    [[nodiscard]] inline virtual std::shared_ptr<Class> getType(const std::string &) const { return nullptr; }

    [[nodiscard]] inline virtual std::shared_ptr<function_base_t> getFunction(const std::string &) const {
      return nullptr;
    }

    [[nodiscard]] inline size_t getMetadataSize() const override { return sizeof(*this); }
    [[nodiscard]] inline virtual std::vector<std::shared_ptr<Class> > getTypes() const { return {}; }
    [[nodiscard]] inline virtual std::vector<std::shared_ptr<function_base_t> > getFunctions() const { return {}; }
    inline void setLuaDumpPrefix(const std::string &prefix) { _luaDumpPrefix = prefix; }
  };

  class BuiltInAssembly : public Assembly {
  private:
    std::unordered_map<std::string, std::shared_ptr<Class> > namedClasses;
    std::unordered_map<std::type_index, std::shared_ptr<Class> > indexedClasses;
    std::unordered_map<std::string, std::shared_ptr<function_base_t> > functions;

    template<typename T>
    inline void registerType(const std::shared_ptr<Class> &cls) {
      this->namedClasses[std::string(nameof<T>())] = cls;
      this->indexedClasses[cls->cindex()] = cls;
    }

    template<typename T>
    void addExtents() {
      using P = T *;
      using CP = const T *;
      using R = T &;
      using CR = const T &;

      registerType<P>(make_class<P>()
        .method("ref", [](P p) -> T & { return *p; })
        .method("val", [](P p) -> T { return *p; })
        .method("new", [](const int &len) {
          if constexpr (std::is_default_constructible_v<T>)
            return new T[len];
          else return nullptr;
        })
        .method("del", [](P p) { delete[] p; })
        .done());

      registerType<R>(make_class<R>()
        .method("val", [](R r) -> T { return r; })
        .done());

      registerType<CP>(make_class<CP>()
        .method("ref", [](CP p) -> const T & { return *p; })
        .method("val", [](CP p) -> T { return *p; })
        .method("new", [](const int &len) -> CP {
          if constexpr (std::is_default_constructible_v<T>)
            return new T[len];
          else return nullptr;
        })
        .method("del", [](CP p) { delete[] p; })
        .done());

      registerType<CR>(make_class<CR>()
        .method("val", [](CR r) -> const T { return r; })
        .done());
    }

  public:
    inline size_t getMetadataSize() const override {
      size_t total = sizeof(*this) + getStringRealSize(_name) + sizeof(functions) + sizeof(namedClasses) + sizeof(
                       indexedClasses);
      for (const auto &[k, v]: this->functions) {
        total += getStringRealSize(k);
        total += sizeof(v) + v->getMetadataSize();
      }

      for (const auto &[k, v]: this->namedClasses) {
        total += getStringRealSize(k);
        total += v->getMetadataSize();
        total += sizeof(v);
      }

      for (const auto &[k, v]: this->indexedClasses) {
        total += sizeof(k) + sizeof(v);
      }

      return total;
    }

    inline Object dump() const override {
      return Map()
          .add("name", this->name())
          .add("classes", kits::select(this->namedClasses, [](auto I) { return I.second->dump(); }))
          .add("functions", kits::select(this->functions, [](auto I) { return I.second->dump(); }));
    }

    inline void bind(ul2::lstate &state) const override {
      auto prefix = this->_luaDumpPrefix.empty() ? "" : kits::replace(_luaDumpPrefix, "::", ".") + ".";
      for (const auto &[name, func]: this->functions) {
        state.bind2(kits::replace(prefix + name, "::", "."), func->lua());
      }

      for (const auto &[idx, cl]: this->namedClasses) {
        // NOTE: I don't like how this works so ima use the C++ mangled name for the Lua side. Anyways,
        // I count on making a dump system that will later dump assemblies and be used as assembly glue
        // in order to be able to use full-user-names instead of weird C++ Mangled names.
        std::string classname = prefix + this->_name + "." + cl->cindex().name();
        for (const auto &[name, fun]: cl->methods()) {
          auto maNamePlease = kits::replace(classname + "." + name, "::", ".");
          state.bind2(maNamePlease, fun->lua());
        }
      }
    }

    inline BuiltInAssembly(const std::string &s) : Assembly(s) {
    }

    inline std::string name() const override { return _name; }

    inline std::shared_ptr<Class> getType(const std::type_index &index) const override {
      if (indexedClasses.find(index) == indexedClasses.end()) { return nullptr; }
      return indexedClasses.at(index);
    }

    inline std::shared_ptr<Class> getType(const std::string &name) const override {
      if (namedClasses.find(name) == namedClasses.end()) { return nullptr; }
      return namedClasses.at(name);
    }

    inline std::shared_ptr<function_base_t> getFunction(const std::string &name) const override {
      if (functions.find(name) == functions.end()) { return nullptr; }
      return functions.at(name);
    }

    template<typename T>
    inline BuiltInAssembly &addType(const std::shared_ptr<Class> cls) {
      registerType<T>(cls);

      using U = kits::flat_t<T>;

      if constexpr (!std::is_void_v<U> && kits::well<T>()) {
        addExtents<U>();
      }

      // TODO: std::shared_ptr<U>, std::vector<U>, std::unique_ptr<U> and std::weak_ptr<U>

      return *this;
    }

    inline BuiltInAssembly &addFunction(const std::string &name, const std::shared_ptr<function_base_t> &func) {
      auto myName = this->_name + "::" + name;
      this->functions[myName] = func;
      return *this;
    }

    template<typename... Fs>
    inline BuiltInAssembly &addFunction(const std::string &name, Fs... fs) {
      auto myName = this->_name + "::" + name;
      this->functions[myName] = make_function(myName, std::forward<Fs>(fs)...);
      return *this;
    }

    inline std::vector<std::shared_ptr<Class> > getTypes() const override {
      return kits::select(this->namedClasses, [](auto I) { return I.second; });
    }

    inline std::vector<std::shared_ptr<function_base_t> > getFunctions() const override {
      return kits::select(this->functions, [](auto I) { return I.second; });
    }
  };
}

#endif //EASYRPG_PLAYER_ASSEMBLY_HPP
