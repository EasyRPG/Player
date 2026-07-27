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

#include <string>
#include <utility>
#include <typeinfo>
#include <typeindex>
#include <string_view>

#include "../iky7/nameof.hpp"
#include "../kits/utils.hpp"
#include "metadata.hpp"

namespace leasy::metadata {
  class TypeInfo;

  template <typename T>
  unsigned long long safe_sizeof() {
    if constexpr (std::is_void_v<T>) return 0;
    else {
      return sizeof(T);
    }
  }

  extern void _register_metadata_type(const std::type_info &info, const TypeInfo &metadata);
  
  class TypeInfo final : public Data {
    public:
    using index_type = std::type_index;
    
    TypeInfo()
    : m_index(typeid(void)),
    m_name("void"),
    m_implname(typeid(void).name()) {}
    
    explicit TypeInfo(const std::type_index& info)
    : m_index(info),
    m_name(info.name()),
    m_implname(info.name()) {}
    
    template<typename T>
    static TypeInfo from() {
      auto info = TypeInfo(
        typeid(T),
        nameof<T>(),
        typeid(T).name(),
        safe_sizeof<T>()
      );
      
      _register_metadata_type(typeid(T), info);
      
      return info;
    }
    
    template<typename T>
    TypeInfo(std::in_place_type_t<T>)
      : TypeInfo(
          typeid(T),
          nameof<T>(),
          typeid(T).name(),
          safe_sizeof<T>()
        ) {
          _register_metadata_type(typeid(T), TypeInfo(typeid(T), nameof<T>(), typeid(T).name(), safe_sizeof<T>()));
        }

    [[nodiscard]]
    std::type_index index() const noexcept {
      return m_index;
    }

    [[nodiscard]]
    std::string name() const noexcept {
      return m_name;
    }

    [[nodiscard]]
    std::string implname() const noexcept {
      return m_implname;
    }

    [[nodiscard]]
    std::size_t hash_code() const noexcept {
      return m_index.hash_code();
    }

    explicit operator std::type_index() const noexcept {
      return m_index;
    }

    friend bool operator==(const TypeInfo &lhs, const TypeInfo &rhs) noexcept {
      return (lhs.index() == rhs.index());
    }

    friend bool operator!=(const TypeInfo& lhs, const TypeInfo& rhs) noexcept {
      return !(lhs.index() == rhs.index());
    }

    friend bool operator<(const TypeInfo& lhs, const TypeInfo& rhs) noexcept {
      return lhs.m_index < rhs.m_index;
    }

    size_t size() const {
      return m_size;
    }

    Object dump() const override {
      return Map().add("name", this->name())
                    .add("implname", this->implname())
                    .add("hash", this->hash_code())
                    .add("size", this->size());
    }

  private:
    TypeInfo(
      const std::type_info& info,
      std::string_view user_name,
      std::string_view impl_name,
      size_t size__ = 0)
      : m_index(info),
        m_name(user_name),
        m_implname(impl_name) {}

    std::type_index m_index;
    std::string m_name;
    std::string m_implname;
    size_t m_size = 0;
  };

  template<typename T>
  inline const TypeInfo typeinfo_v = TypeInfo::from<T>();
} // namespace leasy::metadata

template <typename T>
leasy::metadata::TypeInfo typeidof() {
  return leasy::metadata::TypeInfo::from<T>();
}

template <typename T>
leasy::metadata::TypeInfo typeidof(T &&) {
  return typeidof<T>();
}
