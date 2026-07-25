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

#include <variant>
#include <functional>

namespace leasy::kits { 
  template<typename... Ts>
  class variant {
  protected:
    std::variant<Ts...> value;

  public:
    variant() : value(std::in_place_index<0>) {}

    template<typename T>
    variant(T&& v)
      : value(std::forward<T>(v)) {}

    template<typename T, typename F>
    variant& when(F&& f) & {
      if (auto* p = std::get_if<T>(&value))
      std::forward<F>(f)(*p);
      return *this;
    }
    
    template<typename T, typename F>
    variant&& when(F&& f) && {
      if (auto* p = std::get_if<T>(&value))
      std::forward<F>(f)(std::move(*p));
      return std::move(*this);
    }


    template<typename T, typename F>
    const variant& when(F&& f) const& {
      if (auto const* p = std::get_if<T>(&value))
      std::forward<F>(f)(*p);
      return *this;
    }
    
    template<typename T, typename F>
    const variant&& when(F&& f) const&& {
      if (auto const* p = std::get_if<T>(&value))
      std::forward<F>(f)(std::move(*p));
      return std::move(*this);
    }
    
    template <typename T>
    variant& set(T&& v) {
      value = std::forward<T>(v);
      return *this;
    }

    const std::variant<Ts...>& internal() const {
      return this->value;
    }

    std::variant<Ts...>& internal() {
      return this->value;
    }
  };
}