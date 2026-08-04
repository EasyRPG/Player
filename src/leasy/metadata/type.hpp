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

#include <memory>
#include <string>
#include <utility>
#include <typeinfo>
#include <typeindex>
#include <string_view>

#include "../iky7/nameof.hpp"
#include "../kits/utils.hpp"
#include "metadata.hpp"
#include "function_base.hpp"
#include "leasy/kits/select.hpp"

namespace leasy::metadata {
  class Class;
  template <typename... Fs>
  inline std::shared_ptr<function_base_t>  make_function(const std::string&, Fs&&...);

  template<typename T, typename = void>
  struct sizeof_or_zero : std::integral_constant<std::size_t, 0> {};

  template<typename T>
  struct sizeof_or_zero<T, std::void_t<decltype(sizeof(T))>>
    : std::integral_constant<std::size_t, sizeof(T)> {};

  template<typename T>
  constexpr std::size_t safe_sizeof()
  {
    return sizeof_or_zero<T>::value;
  }

  std::shared_ptr<Class> typeidof(const std::type_index&);
  
  class Class : public Data {
  protected:
    std::type_index _cindex;
    std::unordered_map<std::string, std::shared_ptr<function_base_t>> _methods;
    std::vector<std::shared_ptr<Class>> _bases;
    size_t _size;
    std::string _fullname;

  public:
    inline virtual ~Class() {}

    inline virtual std::any call(const std::string &name, std::vector<std::any> &args) const {
      if (_methods.find(name) != _methods.end()) return _methods.at(name)->call(args);
      else {
        std::exception *ex = nullptr;
        for (const auto &base: _bases) {
          if (!base) continue;

          try {
            return base->call(name, args);
          } catch (const std::exception &e) {
            *ex = std::move(e);
          }
        }

        throw ex ? std::runtime_error(ex->what()) : std::runtime_error(name + ": method not found in base " + _fullname);
      }
    }

    inline std::shared_ptr<function_base_t> get_method(const std::string &name) const {
      if (const auto it = _methods.find(name); it != _methods.end()) return it->second;

      for (auto &base : _bases) {
        if (!base) continue;

        auto fn = base->get_method(name);
        if (fn)
          return fn;
      }

      return nullptr;
    }

    inline virtual std::any activate(std::vector<std::any> &args) const {
      return call("activate", args);
    }

    inline virtual std::string fullname() const { return _fullname; }
    inline virtual std::type_index cindex() const { return _cindex; }
    inline virtual size_t size() const { return _size; }
    inline virtual std::vector<std::shared_ptr<Class>> bases() const { return _bases; }

    inline virtual std::unordered_map<std::string, std::shared_ptr<function_base_t>> methods() const {
      auto result = _methods;
      for (auto &base: _bases) {
        for (auto &[name, method]: base->methods()) {
          result.emplace(name, method);
        }
      }

      return result;
    }

    inline Object dump() const override {
      return Map()
        .add("name", _fullname)
        .add("size", _size)
        .add("bases", Array(
          kits::select(this->_bases, [](const std::shared_ptr<Class> &cl) { 
            if (cl) return cl->dump();
            return Object("<null-class>");
          })
        ))
        .add("cindex", Map()
          .add("name", _cindex.name())
          .add("hashcode", _cindex.hash_code())
        )
        .add("methods", Array(kits::select(this->methods(), [](const std::pair<std::string, std::shared_ptr<function_base_t>> &e) {
          return e.second->dump();
        })));
    }

    inline Object minimal_dump() const {
      return Map()
        .add("name", _fullname)
        .add("cindex", Map()
          .add("name", _cindex.name())
          .add("hashcode", _cindex.hash_code())
        );
    }

    //inline void bind(ul2::lstate &state) const override {
    //  for (const auto &[name, func]: this->_methods) {
    //    state.bind2(name, func->lua());
    //  }
    //}

    inline Class() : _cindex(typeid(void)), _size(0), _fullname("void") {
      /** Null class creation */
    }

    template <typename T>
    inline static std::shared_ptr<Class> from() {
      return typeidof(typeid(T));
    }
  };

  class UnresolvedClass : public Class {
  public:
    inline UnresolvedClass(const std::type_index &poor_data) {
      _fullname = poor_data.name();
      _cindex = poor_data;
    }
    
    inline UnresolvedClass(const std::type_index &poor_data, const std::string &ful) {
      _fullname = ful;
      _cindex = poor_data;
    }

    inline Object dump() const override {
      auto resolved = typeidof(_cindex);

      if (resolved.get() != this && !std::dynamic_pointer_cast<UnresolvedClass>(resolved)) {
        return resolved->dump();
      }

      return Map()
          .add("name", _fullname)
          .add("cindex", Map().add("name", _cindex.name()).add("hash", _cindex.hash_code()))
          .add("resolved", false);
    }
  };

  template <typename T>
  inline void _remember_names_please() {
    using U = kits::flat_t<T>;
    nameof<U>();
    nameof<T>(); // hahahaha so it caches everything yk
    if constexpr (! std::is_void_v<U>) {
      nameof<U*>();
      nameof<U&>();
      nameof<const U>();
      nameof<const U*>();
      nameof<const U&>();
    }
  }

  template <typename T>
  std::shared_ptr<leasy::metadata::Class> typeidof() {
    _remember_names_please<T>();
    return typeidof(typeid(T));
  }

  void _make_type(const std::type_index&, const std::shared_ptr<Class>&);

  /** @brief a dynamic class, made at runtime. */
  template <typename T, typename... Bases>
  class DynamicClass : public Class {
  public:
    inline DynamicClass() {
      this->_cindex = typeid(T);
      this->_fullname = nameof<T>();
      this->_size = safe_sizeof<T>();
      this->_bases = {typeidof<Bases>()...};
    }

    template <typename... Fs>
    inline DynamicClass &method(const std::string &name, Fs&&... funcs) {
      this->_methods[name] = make_function(name, std::forward<Fs>(funcs)...);
      return *this;
    }

    inline std::shared_ptr<Class> done() const {
      return std::make_shared<DynamicClass>(*this);
    }
  };

  /** @brief makes class registration easier, wrapping around a DynamicClass. */
  template <typename T, typename... Bases>
  class ClassBuilder {
  protected:
    std::shared_ptr<DynamicClass<T, Bases...>> local;
    
    static inline std::shared_ptr<DynamicClass<T, Bases...>> alloc_ptr() {
      std::shared_ptr<DynamicClass<T, Bases...>> p = std::make_shared<DynamicClass<T, Bases...>>();
      _make_type(typeid(T), p);
      return p;
    }

  public:
    inline ClassBuilder() {
      nameof<bool>();
      this->local = alloc_ptr();
      _remember_names_please<T>();
      using U = std::remove_reference_t<T>;
      if constexpr (! std::is_void_v<U>) {
        this->local->method("ptr", [](U &i) { return &i; });
      }
    }

    template <typename... Fs>
    inline ClassBuilder<T, Bases...> &method(const std::string &name, Fs&&... funcs) {
      this->local->method(name, std::forward<Fs>(funcs)...);
      return *this;
    }

    inline std::shared_ptr<Class> done() const {
      return this->local;
    }
  };

  template <typename T, typename... Bases>
  inline ClassBuilder<T, Bases...> make_class() {
    return ClassBuilder<T, Bases...>();
  }

  bool is_same_type(const std::type_index &cindex, const std::shared_ptr<Class> &classptr);
} // namespace leasy::metadata

template <typename T>
std::shared_ptr<leasy::metadata::Class> typeidof() {
  return leasy::metadata::typeidof(typeid(T));
}

inline std::shared_ptr<leasy::metadata::Class> typeidof(const std::type_index &idx) {
  return leasy::metadata::typeidof(idx);
}