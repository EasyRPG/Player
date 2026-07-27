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
 *  0xEF9087A
 * 
 * **********************************************************************/

#pragma once

#include "../ul2/function_traits.hpp"

#include <type_traits>
#include <typeindex>

namespace leasy::typings2 {
  struct unknown_t {};
  
  template<typename T, typename = void>
  struct is_complete : std::false_type {};
  
  template<typename T>
  struct is_complete<T, std::void_t<decltype(sizeof(T))>>
  : std::true_type {};
  
  template<typename T>
  inline constexpr bool is_complete_v = is_complete<T>::value;
  
  template <typename T>
  std::type_index leasy_typeid() {
    if constexpr (is_complete_v<T>) return std::type_index(typeid(T));
    return std::type_index(typeid(unknown_t));
  }
  
  template <template <typename...> typename T>
  std::type_index leasy_typeid() {
    return std::type_index(typeid(unknown_t));
  }
  
  template <typename T, typename... Args>
  auto activate(Args&&... args) {
    using U = std::remove_const_t<std::remove_reference_t<T>>;
    
    if constexpr (
      is_complete_v<U> &&
      std::is_constructible_v<U, Args...> &&
      !std::is_abstract_v<U>
    ) {
      return new U(std::forward<Args>(args)...);
    } else {
      return static_cast<U*>(nullptr);
    }
  }
  
  template<typename T, bool = is_complete_v<T>> 
  struct is_nonabstract : std::true_type {};
  
  template<typename T>
  struct is_nonabstract<T, true> : std::bool_constant<!std::is_abstract_v<T>> {};
  
  template<typename T>
  inline constexpr bool can_store_by_value = is_complete_v<T> && is_nonabstract<T>::value && std::is_copy_constructible_v<T>;

  template <typename T>
  using _leasy_ensure_type = T;

  template<typename Tuple>
  struct tuple_all_storable;

  template<typename... Ts>
  struct tuple_all_storable<std::tuple<Ts...>> 
    : std::bool_constant<(can_store_by_value<std::remove_reference_t<std::remove_const_t<Ts>>> && ...)> {};

  template<typename Tuple>
  inline constexpr bool tuple_all_storable_v = tuple_all_storable<Tuple>::value;

  template<class Param, class T>
  decltype(auto) pass(T&& value) {
    using P = std::remove_cv_t<std::remove_reference_t<Param>>;

    if constexpr (! (is_complete_v<T> || is_complete_v<Param>)) {
      return std::forward<T>(value);
    } else if constexpr (
      std::is_same_v<P, std::remove_cv_t<std::remove_reference_t<T>>> &&
      std::is_move_constructible_v<P> &&
      !std::is_copy_constructible_v<P>
    ) {
      // destination takes a move-only object by value
      return std::move(value);
    } else {
       // preserve whatever the caller gave us
       return std::forward<T>(value);
    }
  }

  template <auto F, typename... Args>
  auto call(Args&&... args) {    
    using R     = typename ul2::function_traits<decltype(F)>::return_type;
    using Tuple = typename ul2::function_traits<decltype(F)>::args_tuple;

    if constexpr (!tuple_all_storable_v<Tuple>) {
      throw std::runtime_error("Function contains non-storable parameter type.");
    }

    return std::invoke(F, std::forward<Args>(args)...);
  }
}

