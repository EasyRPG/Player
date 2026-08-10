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

#include <tuple>
#include <vector>
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace leasy::kits {
  template <typename T>
  using flat_t = std::remove_reference_t<std::remove_cv_t<T>>;

  template<typename T>
  struct is_vector : std::false_type {};

  template<typename T, typename A>
  struct is_vector<std::vector<T, A> > : std::true_type {};

  template<typename T>
  struct is_unordered_map : std::false_type {};

  template<typename K, typename V, typename H, typename E, typename A>
  struct is_unordered_map<std::unordered_map<K, V, H, E, A> > : std::true_type {};

  template<typename T> struct is_shared_ptr : std::false_type {};

  template<typename T> struct is_shared_ptr<std::shared_ptr<T> > : std::true_type {};

  template <typename... Ts> struct is_tuple : std::false_type {};

  template <typename... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type {};

  template<typename T> inline constexpr bool  is_vector_v = is_vector<T>::value;
  template <typename T> inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;
  template <typename T> inline constexpr bool is_unordered_map_v = is_unordered_map<T>::value;
  template <typename T> inline constexpr bool is_tuple_v = is_tuple<T>::value;

  template<typename T>
  struct vector_value_type;

  template<typename U, typename Alloc>
  struct vector_value_type<std::vector<U, Alloc>> {
    using type = U;
  };

  template<typename T>
  using vector_value_t = typename vector_value_type<flat_t<T>>::type;

  template<class T>
  struct underlying {
    using type = T;
  };

  template<class T>
  struct underlying<T*> {
    using type = typename underlying<T>::type;
  };

  template<class T>
  struct underlying<T&> {
    using type = typename underlying<T>::type;
  };

  template<class T>
  struct underlying<T&&> {
    using type = typename underlying<T>::type;
  };

  template<class T>
  struct underlying<const T> {
    using type = typename underlying<T>::type;
  };

  template<class T>
  struct underlying<volatile T> {
    using type = typename underlying<T>::type;
  };

  template<class T>
  struct underlying<const volatile T> {
    using type = typename underlying<T>::type;
  };

  template<class T>
  using underlying_t = typename underlying<T>::type;
}