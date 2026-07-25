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
#include <memory>
#include <cstdint>
#include <unordered_map>

#include "../kits/variant.hpp"

namespace leasy::metadata {
  struct  null {};
  using   integer = int64_t;
  using   uinteger = uint64_t;
  using   number = long double;
  using   boolean = bool;
  using   string = std::string;

  class   Object;

  class Array {
  protected:
    std::vector<std::shared_ptr<Object>> values_;

  public:
    inline Array() {}
    inline Array(const std::vector<Object>&);
    inline std::vector<Object> values() const;
    inline Array& add(const Object&);
    inline Object& operator[](size_t pos);
    const Object& operator[](size_t pos) const;
  };

  class Map {
  protected:
    std::unordered_map<std::string, std::shared_ptr<Object>> data;

  public:
    inline std::unordered_map<std::string, Object> values() const;
    inline Map& add(const std::string&, const Object&);
    inline Object& operator[](const std::string&);
    const Object& operator[](const std::string&) const;
  };

  class Object 
    : public kits::variant<
      null, integer, uinteger, number, boolean, string, Array, Map
    > {
  public:
    using kits::variant<null, integer, uinteger, number, boolean, string, Array, Map>::variant;
    using kits::variant<null, integer, uinteger, number, boolean, string, Array, Map>::set;
    using kits::variant<null, integer, uinteger, number, boolean, string, Array, Map>::when; 
  };
  
  inline Array::Array(const std::vector<Object> &objs) {
    for (const auto& obj: objs) {
      this->values_.push_back(std::make_shared<Object>(obj));
    }
  }

  inline std::vector<Object> Array::values() const {
    std::vector<Object> values;
    values.reserve(this->values_.size());

    for (const auto &val: this->values_) {
      values.push_back(*val);
    }

    return values;
  }

  inline Array& Array::add(const Object &val) {
    this->values_.emplace_back(std::make_shared<Object>(val));
    return *this;
  }

  inline Object& Array::operator[](size_t pos) {
    // C++'s standard vector will handle exceptions for me hihi.
    return *(this->values_[pos]);
  }
  
  inline const Object& Array::operator[](size_t pos) const {
    return *(this->values_[pos]);
  }

  inline std::unordered_map<std::string, Object> Map::values() const {
    std::unordered_map<std::string, Object> map;

    for (const auto &[K, V]: this->data) {
      map[K] = *V;
    }

    return map;
  }

  inline Map& Map::add(const std::string &k, const Object &v) {
    this->data[k] = std::make_shared<Object>(v);
    return *this;
  }

  inline Object& Map::operator[](const std::string &k) {
    auto &ptr = this->data[k];

    if (!ptr)
      ptr = std::make_shared<Object>();

    return *ptr;
  }
  
  inline const Object& Map::operator[](const std::string &k) const {
    return *(this->data.at(k));
  }
}