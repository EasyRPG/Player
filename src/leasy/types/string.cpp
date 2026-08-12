//
// Created by @wys on 12/08/2026.
//

#include <functional>
#include <string>
#include "string.hpp"
#include "leasy/metadata/Domain.hpp"

namespace {
  using namespace leasy;

  template<typename StringT>
  auto registerStringFunctions(
    metadata::BuiltInAssembly &assembly
  ) {
    using T = StringT;

    assembly.addType<StringT>(metadata::make_class<StringT>()
        .method("size", [](const T &self) {
          return self.size();
        })
        .method("empty", [](const T &self) {
          return self.empty();
        })
        .method("hasValue", [](const T &self, typename T::char_type value) {
          return self.hasValue(value);
        })
        .method("hasIndex", [](const T &self, size_t index) {
          return self.hasIndex(index);
        })
        .method("lower", [](const T &self) {
          return self.lower();
        })
        .method("upper", [](const T &self) {
          return self.upper();
        })
        .method("lowerInPlace", [](T &self) -> T & {
          return self.lowerInPlace();
        })
        .method("upperInPlace", [](T &self) -> T & {
          return self.upperInPlace();
        })
        .method("replace", [](T &self, const T &from, const T &to) -> T & {
          return self.replace(from, to);
        })
        .method("substr", [](const T &self, size_t start, size_t length) {
          return self.substr(start, length);
        }).done());

    return assembly;
  }


  template<
    typename CharT,
    typename Traits = std::char_traits<CharT>,
    typename Allocator = std::allocator<CharT>
  >
  void registerBasicStringFunctions(
    metadata::BuiltInAssembly &assembly
  ) {
    using String = std::basic_string<CharT, Traits, Allocator>;

    assembly.addType<String>(metadata::make_class<String>()
        .method(
          "size",
          [](const String &self) -> typename String::size_type {
            return self.size();
          }
        )

        .method(
          "length",
          [](const String &self) -> typename String::size_type {
            return self.length();
          }
        )

        .method(
          "empty",
          [](const String &self) -> bool {
            return self.empty();
          }
        )

        .method(
          "capacity",
          [](const String &self) -> typename String::size_type {
            return self.capacity();
          }
        )

        .method(
          "maxSize",
          [](const String &self) -> typename String::size_type {
            return self.max_size();
          }
        )

        .method(
          "clear",
          [](String &self) -> void {
            self.clear();
          }
        )

        .method(
          "front",
          [](const String &self) -> const CharT & {
            return self.front();
          }
        )

        .method(
          "back",
          [](const String &self) -> const CharT & {
            return self.back();
          }
        )

        .method(
          "at",
          [](const String &self, typename String::size_type index)
        -> const CharT & {
            return self.at(index);
          }
        )

        .method(
          "find",
          [](const String &self, const String &value)
        -> typename String::size_type {
            return self.find(value);
          }
        )

        .method(
          "findChar",
          [](const String &self, CharT value)
        -> typename String::size_type {
            return self.find(value);
          }
        )

        .method(
          "rfind",
          [](const String &self, const String &value)
        -> typename String::size_type {
            return self.rfind(value);
          }
        )

        .method(
          "startsWith",
          [](const String &self, const String &value) -> bool {
            if (value.size() > self.size())
              return false;

            return self.compare(0, value.size(), value) == 0;
          }
        )

        .method(
          "endsWith",
          [](const String &self, const String &value) -> bool {
            if (value.size() > self.size())
              return false;

            return self.compare(
                     self.size() - value.size(),
                     value.size(),
                     value
                   ) == 0;
          }
        )

        .method(
          "contains",
          [](const String &self, const String &value) -> bool {
            return self.find(value) != String::npos;
          }
        )

        .method(
          "substr",
          [](const String &self,
             typename String::size_type start,
             typename String::size_type length)
        -> String {
            return self.substr(start, length);
          }
        )

        .method(
          "compare",
          [](const String &self, const String &other) -> int {
            return self.compare(other);
          }
        )

        .method(
          "append",
          [](String &self, const String &other) -> String & {
            return self.append(other);
          }
        )

        .method(
          "pushBack",
          [](String &self, CharT value) -> void {
            self.push_back(value);
          }
        )

        .method(
          "popBack",
          [](String &self) -> void {
            self.pop_back();
          }
        )

        .method(
          "replace",
          [](String &self,
             typename String::size_type position,
             typename String::size_type count,
             const String &replacement)
        -> String & {
            return self.replace(position, count, replacement);
          }
        )

        .method(
          "erase",
          [](String &self,
             typename String::size_type position,
             typename String::size_type count)
        -> String & {
            return self.erase(position, count);
          }
        )

        .method(
          "insert",
          [](String &self,
             typename String::size_type position,
             const String &value)
        -> String & {
            return self.insert(position, value);
          }
        )

        .method(
          "resize",
          [](String &self,
             typename String::size_type size)
        -> void {
            self.resize(size);
          }
        )

        .method(
          "reserve",
          [](String &self,
             typename String::size_type size)
        -> void {
            self.reserve(size);
          }
        )

        .method(
          "swap",
          [](String &self, String &other) -> void {
            self.swap(other);
          }
        ).done());
  }

  template <typename T>
  void impl(metadata::BuiltInAssembly &a) {
    registerBasicStringFunctions<T>(a);
    registerStringFunctions<BasicString<T>>(a);
  }

  template <typename... Ts>
  void make(metadata::BuiltInAssembly &a) {
    (impl<Ts>(a), ...);
  }

  bool ok = []() {
    auto assm = metadata::AppDomain().getAssemblyOrCreate<metadata::BuiltInAssembly>("leasy::encoding");

    make<
      char,
      wchar_t,
      char16_t,
      char32_t
    >(*assm);

    return false;
  }();
}
