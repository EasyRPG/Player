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

  template<typename T>
  std::shared_ptr<Class> typeidof();

  class Class : public Data, public SizeDescriptor {
  protected:
    std::type_index _cindex;
    std::unordered_map<std::string, std::shared_ptr<function_base_t>> _methods;
    std::unordered_map<std::string, std::vector<String>> _enums;
    std::vector<std::shared_ptr<Class> > _bases;
    size_t _size;
    String _fullname;
    bool _resolved = true;

  public:

    virtual std::any call(const std::string &name, std::vector<std::any> &args) const {
      if (_methods.find(name) != _methods.end()) return _methods.at(name)->call(args);
      else {
        std::exception_ptr ex = nullptr;
        for (const auto &base: _bases) {
          if (!base) continue;

          try {
            return base->call(name, args);
          } catch (const std::exception &e) {
            ex = std::current_exception();
          }
        }

        if (ex) std::rethrow_exception(ex);
        throw std::runtime_error(name + ": method not found in base " + _fullname);
      }
    }

    std::shared_ptr<function_base_t> getMethod(const std::string &name) const {
      if (const auto it = _methods.find(name); it != _methods.end()) return it->second;

      for (auto &base: _bases) {
        if (!base) continue;

        auto fn = base->getMethod(name);
        if (fn)
          return fn;
      }

      return nullptr;
    }

    inline virtual std::any activate(std::vector<std::any> &args) const {
      return call("activate", args);
    }

    virtual String fullname() const { return _fullname; }
    virtual std::type_index cindex() const { return _cindex; }
    virtual size_t size() const { return _size; }
    virtual std::vector<std::shared_ptr<Class> > bases() const { return _bases; }

    virtual std::unordered_map<std::string, std::shared_ptr<function_base_t> > methods() const {
      auto result = _methods;
      for (auto &base: _bases) {
        for (auto &[name, method]: base->methods()) {
          result.emplace(name, method);
        }
      }

      return result;
    }

    bool isSameAs(const Class &other) const {
      return cindex() == other.cindex();
    }

    bool isBaseOf(const Class &derived) const {
      if (cindex() == derived.cindex())
        return false;

      for (const auto &base: derived._bases) {
        if (!base)
          continue;

        if (base->cindex() == cindex() || isBaseOf(*base))
          return true;
      }

      return false;
    }

    bool isDerivedFrom(const Class &base) const {
      return base.isBaseOf(*this);
    }

    bool isAssignableTo(const Class &target) const {
      // Exact type or implicit upcast.
      return isSameAs(target) || target.isBaseOf(*this);
    }

    bool isAssignableFrom(const Class &source) const {
      return source.isAssignableTo(*this);
    }

    bool isConstructibleFrom(const Class &source) const {
      // "Can an object of source type be used where this type is expected?"
      return isAssignableFrom(source);
    }

    bool isConstructibleTo(const Class &target) const {
      return target.isConstructibleFrom(*this);
    }

    bool isRelatedTo(const Class &other) const {
      return isSameAs(other)
             || isBaseOf(other)
             || other.isBaseOf(*this);
    }

    bool isStrictBaseOf(const Class &derived) const {
      return cindex() != derived.cindex() && isBaseOf(derived);
    }

    bool isStrictDerivedFrom(const Class &base) const {
      return cindex() != base.cindex() && isDerivedFrom(base);
    }

    bool isRoot() const {
      return _bases.empty();
    }

    bool hasBases() const {
      return !_bases.empty();
    }

    bool hasBase(const Class &base) const {
      return isBaseOf(base);
    }

    template<typename T>
    bool isSameAs() const {
      return cindex() == typeid(T);
    }

    template<typename T>
    bool isBaseOf() const {
      return isBaseOf(*typeidof<T>());
    }

    template<typename T>
    bool isDerivedFrom() const {
      return isDerivedFrom(*typeidof<T>());
    }

    template<typename T>
    bool isAssignableTo() const {
      return isAssignableTo(*typeidof<T>());
    }

    template<typename T>
    bool isAssignableFrom() const {
      return isAssignableFrom(*typeidof<T>());
    }

    template<typename T>
    bool isConstructibleFrom() const {
      return isConstructibleFrom(*typeidof<T>());
    }

    template<typename T>
    bool isConstructibleTo() const {
      return isConstructibleTo(*typeidof<T>());
    }

    template<typename T>
    bool isRelatedTo() const {
      return isRelatedTo(*typeidof<T>());
    }

    Object dump() const override {
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

    Object minimalDump() const {
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
    static std::shared_ptr<Class> from() {
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
    UnresolvedClass(const std::type_index &poor_data) {
      _fullname = poor_data.name();
      _cindex = poor_data;
      _resolved = false;
    }

    UnresolvedClass(const std::type_index &poor_data, const std::string &ful) {
      _fullname = ful;
      _cindex = poor_data;
      _resolved = false;
    }

    Object dump() const override {
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
    DynamicClass() {
      this->_cindex = typeid(T);
      this->_fullname = nameof<T>();
      this->_size = safe_sizeof<T>();
      this->_bases = {typeidof<Bases>()...};
    }

    template<typename... Fs>
    DynamicClass &method(const std::string &name, Fs &&... funcs) {
      this->_methods[name] = make_function(name, std::forward<Fs>(funcs)...);
      return *this;
    }

    DynamicClass &enu(const String &name, const std::vector<String> &en) {
      this->_enums[name] = en;
      return *this;
    }

    std::shared_ptr<Class> done() const {
      return std::make_shared<DynamicClass>(*this);
    }
  };

  /** @brief makes class registration easier, wrapping around a DynamicClass. */
  template<typename T, typename... Bases>
  class ClassBuilder {
  protected:
    std::shared_ptr<DynamicClass<T, Bases...> > local;

  private:
    void makeMetadata() {
      this->local->method("type", [] { return std::type_index(typeid(T)); });
      this->local->method("isAbstract", [] { return std::is_abstract_v<T>; });
      this->local->method("isArray", []{ return std::is_array_v<T>; });
      this->local->method("isAggregate", []{ return std::is_aggregate_v<T>; });
      this->local->method("isArithmetic", []{ return std::is_arithmetic_v<T>; });
      this->local->method("isClass", []{ return std::is_class_v<T>; });
      this->local->method("isConst", []{ return std::is_const_v<T>; });
      this->local->method("isFinal", []{ return std::is_final_v<T>; });
      this->local->method("isFloatingPoint", []{ return std::is_floating_point_v<T>; });
      this->local->method("isPolymorphic", []{ return std::is_polymorphic_v<T>; });
      this->local->method("isReference", []{ return std::is_reference_v<T>; });
      this->local->method("isStandardLayout", []{ return std::is_standard_layout_v<T>; });
      this->local->method("isConstructible", []{ return std::is_constructible_v<T>; });
      this->local->method("isCopyConstructible", []{ return std::is_copy_constructible_v<T>; });
      this->local->method("isDefaultConstructible", []{ return std::is_default_constructible_v<T>; });
      this->local->method("isMoveConstructible", []{ return std::is_move_constructible_v<T>; });
      this->local->method("isTriviallyConstructible", []{ return std::is_trivially_constructible_v<T>; });
      this->local->method("isTriviallyMoveConstructible", []{ return std::is_trivially_move_constructible_v<T>; });
      this->local->method("isNothrowConstructible", []{ return std::is_nothrow_constructible_v<T>; });
      this->local->method("isNothrowMoveConstructible", []{ return std::is_nothrow_move_constructible_v<T>; });
    }

  public:
    inline ClassBuilder() {
      this->local = std::make_shared<DynamicClass<T, Bases...> >();

      if constexpr (! std::is_abstract_v<T> && kits::well<T>()) {
        using U = std::remove_reference_t<T>;

        if constexpr (!std::is_void_v<U>) {
          this->local->method("ptr", [](U &i) { return &i; }, [](const U &i) { return &i; });
          this->local->method("ref", [](U &i) -> U& { return i; }, [](const U &i) -> const U & { return i; });
          this->local->method("makeShared", [](U i) {
            return std::make_shared<U>(i);
          });
        }

        if constexpr (std::is_destructible_v<T>) {
          this->local->method("destroy", [](U &self) {
            self.~U();
          });
        }

        if constexpr (std::is_copy_assignable_v<T>) {
          this->local->method("copyTo", [](const U &self, U &to) { to = self;});
          this->local->method("copyFrom", [](U &self, const U &from) { self = from;});
        }

        if constexpr (std::is_move_assignable_v<T>) {
          this->local->method("moveTo", [](U &self, U &to) { to = std::move(self);});
          this->local->method("moveFrom", [](U &self, U &from) { self = std::move(from);});
        }

        if constexpr(std::is_default_constructible_v<T>) {
          this->local->method("constructDefault", []{ return T{}; });
        }

        if constexpr (std::is_copy_constructible_v<T>) this->local->method("makeCopy", [](T self) -> T { return self;});
      }

      makeMetadata();
    }

    template<typename... Fs>
    ClassBuilder &method(const std::string &name, Fs &&... funcs) {
      this->local->method(name, std::forward<Fs>(funcs)...);
      return *this;
    }

    std::shared_ptr<Class> done() const {
      return this->local;
    }

    ClassBuilder &enu(const String &name, const std::vector<String> &en) {
      return this->local->enu(name, en);
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
