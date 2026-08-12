//
// Created by @wys on 09/08/2026.
//

#pragma once

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "iterable.hpp"

namespace leasy {

template<
    typename C,
    typename Traits = std::char_traits<C>,
    typename Allocator = std::allocator<C>
>
class BasicString
    : public std::basic_string<C, Traits, Allocator>,
      public Iterable<size_t, C> {

public:
  using string_type = std::basic_string<C, Traits, Allocator>;
  using char_type = C;
  using traits_type = Traits;
  using allocator_type = Allocator;
  using size_type = typename string_type::size_type;

  using string_type::string_type;

  inline BasicString(const std::string &s)
    : string_type(s) {}

  void forEach(
      const std::function<void(const size_t&, const C&)>& callback
  ) const override {
    for (size_t i = 0; i < string_type::size(); ++i) {
      callback(i, (*this)[i]);
    }
  }

  bool hasValue(const C& value) const override {
    return this->find(value) != string_type::npos;
  }

  [[nodiscard]] bool hasIndex(const size_t& index) const override {
    return index < string_type::size();
  }

  BasicString& replace(
      const BasicString& from,
      const BasicString& to
  ) {
    if (from.empty())
      return *this;

    size_t startPos = 0;

    while ((startPos = this->find(from, startPos)) != string_type::npos) {
      string_type::replace(
          startPos,
          from.length(),
          to
      );

      startPos += to.length();
    }

    return *this;
  }

  BasicString& replace(
      const C* from,
      const C* to
  ) {
    return replace(
        BasicString(from),
        BasicString(to)
    );
  }

  BasicString lower() const {
    BasicString result = *this;

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](C c) {
          return static_cast<C>(
              std::tolower(static_cast<unsigned char>(c))
          );
        }
    );

    return result;
  }

  BasicString upper() const {
    BasicString result = *this;

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](C c) {
          return static_cast<C>(
              std::toupper(static_cast<unsigned char>(c))
          );
        }
    );

    return result;
  }

  BasicString& lowerInPlace() {
    std::transform(
        this->begin(),
        this->end(),
        this->begin(),
        [](C c) {
          return static_cast<C>(
              std::tolower(static_cast<unsigned char>(c))
          );
        }
    );

    return *this;
  }

  BasicString& upperInPlace() {
    std::transform(
        this->begin(),
        this->end(),
        this->begin(),
        [](C c) {
          return static_cast<C>(
              std::toupper(static_cast<unsigned char>(c))
          );
        }
    );

    return *this;
  }

  std::vector<BasicString> split(C separator = '.') const {
    std::vector<BasicString> result;

    std::basic_stringstream<C, Traits, Allocator> stream(*this);
    BasicString item;

    while (std::getline(stream, item, separator)) {
      if (!item.empty())
        result.push_back(item);
    }

    return result;
  }

  struct SubStringInfo {
    BasicString string;
    size_t index{};
    bool found{};
  };

  SubStringInfo findAny(
      const std::vector<BasicString>& elements
  ) const {
    for (const auto& element : elements) {
      const size_t index = this->find(element);

      if (index != string_type::npos) {
        return {
            .string = element,
            .index = index,
            .found = true
        };
      }
    }

    return {
        .string = {},
        .index = string_type::npos,
        .found = false
    };
  }

  template<typename T>
  static T fromStringOrDefault(
      const BasicString& string
  ) {
    std::basic_istringstream<C, Traits, Allocator> stream(string);
    T value{};

    if ((stream >> value) && (stream >> std::ws).eof())
      return value;

    return T{};
  }

  template<typename T>
  T to() const {
    return fromStringOrDefault<T>(*this);
  }

  BasicString substr(size_t start, size_t length = string_type::npos) const {
    return BasicString(
        this->data() + start,
        std::min(length, this->size() - start)
    );
  }

  [[nodiscard]] size_t size() const override {
    return string_type::size();
  }

  static BasicString join(
      const std::vector<BasicString>& elements,
      const BasicString& separator,
      bool includeLast = false
  ) {
    BasicString result;

    for (size_t i = 0; i < elements.size(); ++i) {
      result += elements[i];

      if (i + 1 < elements.size() || includeLast)
        result += separator;
    }

    return result;
  }

  static BasicString join(
      std::initializer_list<BasicString> elements,
      const BasicString& separator,
      bool includeLast = false
  ) {
    return join(
        std::vector<BasicString>(elements),
        separator,
        includeLast
    );
  }
};

  using String = BasicString<char>;
  using WString = BasicString<wchar_t>;
  using U16String = BasicString<char16_t>;
  using U32String = BasicString<char32_t>;

} // namespace leasy::types