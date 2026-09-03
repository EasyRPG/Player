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
// Created by @wys on 09/08/2026.
//

#pragma once

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "iterable.hpp"
#include "string_view.h"

namespace leasy {
  inline bool isspaceUtf8(char ch) {
    return std::isspace(static_cast<unsigned char>(ch));
  }

  template<
    typename C,
    typename Traits = std::char_traits<C>,
    typename Allocator = std::allocator<C> >
  class BasicString
      : public std::basic_string<C, Traits, Allocator>,
        public Iterable<size_t, C> {
  public:
    using string_type = std::basic_string<C, Traits, Allocator>;
    using view_type = std::basic_string_view<C, Traits>;
    using char_type = C;
    using traits_type = Traits;
    using allocator_type = Allocator;
    using size_type = typename string_type::size_type;

    using string_type::string_type;

    BasicString(const string_type &s) : string_type(s) {
    }

    BasicString(const view_type &view) : string_type(view.data()) {
    }

    void forEach(
      const std::function<void(const size_t &, const C &)> &callback
    ) const override {
      for (size_t i = 0; i < string_type::size(); ++i) {
        callback(i, (*this)[i]);
      }
    }

    bool hasValue(const C &value) const override {
      return this->find(value) != string_type::npos;
    }

    [[nodiscard]] bool hasIndex(const size_t &index) const override {
      return index < string_type::size();
    }

    BasicString &replace(
      const BasicString &from,
      const BasicString &to
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

    BasicString &replace(
      const C *from,
      const C *to
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

    BasicString &lowerInPlace() {
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

    BasicString &upperInPlace() {
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

    static BasicString from(C c) {
      return string_type(1, c);
    }

    struct SubStringInfo {
      BasicString string;
      size_t index{};
      bool found{};
    };

    SubStringInfo findAny(
      const std::vector<BasicString> &elements
    ) const {
      for (const auto &element: elements) {
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
      const BasicString &string
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

    lcf::DBString toDBString() const {
      return lcf::DBString((const char *) this->data(), this->length());
    }

    size_t graphicalSize(size_t (*getGlyphSize)(C)) const {
      size_t total{0};

      for (size_t i = 0; i < this->size(); i++) {
        total += getGlyphSize((*this)[i]);
      }

      return total;
    }

    BasicString fitToWidth(size_t width, size_t (*getGlyphSize)(C)) const {
      if (width == 0 || this->empty())return *this;

      BasicString result;
      size_t lineStart = 0;

      while (lineStart < this->size()) {
        if ((*this)[lineStart] == C('\n')) {
          result += '\n';
          ++lineStart;
          continue;
        }

        size_t currentWidth = 0;
        size_t lastSpace = string_type::npos;
        size_t i = lineStart;

        while (i < this->size()) {
          const C c = (*this)[i];

          if (c == C('\n')) {
            result.append(
              this->data() + lineStart,
              i - lineStart
            );

            result += '\n';
            lineStart = i + 1;
            break;
          }

          const size_t glyphWidth = getGlyphSize(c);

          if (currentWidth + glyphWidth > width) {
            if (lastSpace != string_type::npos &&
                lastSpace >= lineStart) {
              result.append(
                this->data() + lineStart,
                lastSpace - lineStart
              );

              result += '\n';

              lineStart = lastSpace + 1;

              while (lineStart < this->size() &&
                     isspaceUtf8((*this)[lineStart]) &&
                     (*this)[lineStart] != C('\n')) {
                ++lineStart;
              }
            } else {
              result.append(
                this->data() + lineStart,
                i - lineStart
              );

              result += '\n';
              lineStart = i;
            }

            break;
          }

          currentWidth += glyphWidth;

          if (isspaceUtf8(c))lastSpace = i;

          ++i;

          if (i == this->size()) {
            result.append(
              this->data() + lineStart,
              i - lineStart
            );

            lineStart = i;
          }
        }
      }

      return result;
    }

    size_t countOf(const C &ch) const {
      return std::count(this->begin(), this->end(), ch);
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

    [[nodiscard]] bool startsWith(const BasicString &prefix) const {
      return this->size() >= prefix.size() &&
             this->compare(0, prefix.size(), prefix) == 0;
    }

    [[nodiscard]] bool endsWith(const BasicString &suffix) const {
      return this->size() >= suffix.size() &&
             this->compare(
               this->size() - suffix.size(),
               suffix.size(),
               suffix
             ) == 0;
    }

    static BasicString join(
      const std::vector<BasicString> &elements,
      const BasicString &separator,
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
      const BasicString &separator,
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
