#pragma once

#include <tuple>

namespace leasy::ul2 {

template <typename> struct function_traits;

template <typename R, typename... Args> struct function_traits<R (*)(Args...)> {
  using return_type = R;
  using args_tuple = std::tuple<Args...>;

  static constexpr std::size_t arity = sizeof...(Args);
};

} // namespace leasy::ul2
