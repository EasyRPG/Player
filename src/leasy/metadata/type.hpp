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
#include "structs/metadata.hpp"
#include "function_base.hpp"
#include "leasy/kits/select.hpp"

namespace leasy::metadata {
  class Class;

  template<typename... Fs>
  inline std::shared_ptr<function_base_t> make_function(const std::string &, Fs &&...);

  template<typename T, typename = void>
  struct sizeof_or_zero : std::integral_constant<std::size_t, 0> {
  };

  template<typename T>
  struct sizeof_or_zero<T, std::void_t<decltype(sizeof(T))> >
      : std::integral_constant<std::size_t, sizeof(T)> {
  };

  template<typename T>
  constexpr std::size_t safe_sizeof() {
    return sizeof_or_zero<T>::value;
  }

  std::shared_ptr<Class> typeidof(const std::type_index &);

  class Class : public Data, public SizeDescriptor {
  protected:
    std::type_index _cindex;
    std::unordered_map<std::string, std::shared_ptr<function_base_t> > _methods;
    std::vector<std::shared_ptr<Class> > _bases;
    size_t _size;
    std::string _fullname;
    bool _resolved = true;

  public:
    inline virtual std::any call(const std::string &name, std::vector<std::any> &args) const {
      if (_methods.find(name) != _methods.end()) return _methods.at(name)->call(args);
      else {
        std::exception *ex = nullptr;
        for (const auto &base: _bases) {
          if (!base) continue;

          try {
            return base->call(name, args);
          } catch (const std::exception &e) {
            *ex = e;
          }
        }

        throw ex
                ? std::runtime_error(ex->what())
                : std::runtime_error(name + ": method not found in base " + _fullname);
      }
    }

    inline std::shared_ptr<function_base_t> get_method(const std::string &name) const {
      if (const auto it = _methods.find(name); it != _methods.end()) return it->second;

      for (auto &base: _bases) {
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
    inline virtual std::vector<std::shared_ptr<Class> > bases() const { return _bases; }

    inline virtual std::unordered_map<std::string, std::shared_ptr<function_base_t> > methods() const {
      auto result = _methods;
      for (auto &base: _bases) {
        for (auto &[name, method]: base->methods()) {
          result.emplace(name, method);
        }
      }

      return result;
    }

    inline Object dump() const override {
      auto c = resolve();

      return Map()
          .add("name", c->_fullname)
          .add("size", c->_size)
          .add("bases", Array(
      kits::select(c->_bases, [](const std::shared_ptr<Class> &cl) {
               if (cl) return cl->dump();
               return Object("<null-class>");
             })))
          .add("cindex", Map()
            .add("name", c->_cindex.name())
            .add("hashcode", c->_cindex.hash_code())
          )
          .add("methods", Array(kits::select(c->methods(),
            [](const std::pair<std::string, std::shared_ptr<function_base_t> > &e) {
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

    void bind(ul2::lstate &state) const override;

    inline Class() : _cindex(typeid(void)), _size(0), _fullname("void") {
      /** Null class creation */
    }

    template<typename T>
    inline static std::shared_ptr<Class> from() {
      return typeidof(typeid(T));
    }

    inline virtual std::shared_ptr<Class> resolve() const {
      auto type = typeidof(this->cindex());

      if (!type->resolved()) {
        io().Warning.writeln("unable to resolve type ", type->cindex().name(), " (", type->fullname(), ')');
      }

      return type;
    }

    inline bool resolved() const { return this->_resolved; }

    size_t getMetadataSize() const override {
      size_t total = sizeof(*this);

      total += _fullname.capacity();

      total += sizeof(std::shared_ptr<Class>) * _bases.capacity();

      for (auto const &b: _bases)
        if (b)
          total += b->getMetadataSize();

      total += sizeof(
        std::unordered_map<
          std::string,
          std::shared_ptr<function_base_t>
        >);

      total += _methods.bucket_count() * sizeof(void *);

      for (auto const &[name, fn]: _methods) {
        total += name.capacity();

        if (fn) total += fn->getMetadataSize();
      }

      return total;
    }
  };

  class UnresolvedClass : public Class {
  public:
    inline UnresolvedClass(const std::type_index &poor_data) {
      _fullname = poor_data.name();
      _cindex = poor_data;
      _resolved = false;
    }

    inline UnresolvedClass(const std::type_index &poor_data, const std::string &ful) {
      _fullname = ful;
      _cindex = poor_data;
      _resolved = false;
    }

    inline Object dump() const override {
      auto resolved = this->resolve();

      return Map()
          .add("name", _fullname)
          .add("cindex", Map().add("name", _cindex.name()).add("hash", _cindex.hash_code()))
          .add("resolved", false);
    }
  };

  template<typename T>
  std::shared_ptr<leasy::metadata::Class> typeidof() {
    nameof<T>();
    return typeidof(typeid(T));
  }

  /** @brief a dynamic class, made at runtime. */
  template<typename T, typename... Bases>
  class DynamicClass : public Class {
  public:
    inline DynamicClass() {
      this->_cindex = typeid(T);
      this->_fullname = nameof<T>();
      this->_size = safe_sizeof<T>();
      this->_bases = {typeidof<Bases>()...};
    }

    template<typename... Fs>
    inline DynamicClass &method(const std::string &name, Fs &&... funcs) {
      this->_methods[name] = make_function(name, std::forward<Fs>(funcs)...);
      return *this;
    }

    inline std::shared_ptr<Class> done() const {
      return std::make_shared<DynamicClass>(*this);
    }
  };

  /** @brief makes class registration easier, wrapping around a DynamicClass. */
  template<typename T, typename... Bases>
  class ClassBuilder {
  protected:
    std::shared_ptr<DynamicClass<T, Bases...> > local;

  public:
    inline ClassBuilder() {
      this->local = std::make_shared<DynamicClass<T, Bases...> >();
      using U = std::remove_reference_t<T>;

      if constexpr (!std::is_void_v<U>) {
        this->local->method("ptr", [](U &i) { return &i; });
      }
    }

    template<typename... Fs>
    inline ClassBuilder<T, Bases...> &method(const std::string &name, Fs &&... funcs) {
      this->local->method(name, std::forward<Fs>(funcs)...);
      return *this;
    }

    inline std::shared_ptr<Class> done() const {
      return this->local;
    }
  };

  template<typename T, typename... Bases>
  inline ClassBuilder<T, Bases...> make_class() {
    return ClassBuilder<T, Bases...>();
  }

  inline bool is_same_type(const std::type_index &cindex, const std::shared_ptr<Class> &classptr) {
    return classptr->cindex() == cindex;
  }
} // namespace leasy::metadata

template<typename T>
std::shared_ptr<leasy::metadata::Class> typeidof() {
  return leasy::metadata::typeidof(typeid(T));
}

inline std::shared_ptr<leasy::metadata::Class> typeidof(const std::type_index &idx) {
  return leasy::metadata::typeidof(idx);
}
