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

#include "any_function.hpp"
#include "function_traits.hpp"
#include "lua_stack.hpp"

#include <any>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

namespace leasy::ul2 {

template <typename F>
class function_wrapper final : public any_function {
public:
    using traits     = function_traits<remove_cvref_t<F>>;
    using ret_t      = typename traits::return_type;
    using args_tuple = typename traits::args_tuple;

    static constexpr std::size_t arity_v = traits::arity;

    explicit function_wrapper(F fn) : fn_(std::move(fn)) {}

    int invoke_lua(lua_State* L) const override {
        return invoke_lua_impl(L, std::make_index_sequence<arity_v>{});
    }

    std::any invoke_any(const std::vector<std::any>& args) const override {
        return invoke_any_impl(args, std::make_index_sequence<arity_v>{});
    }

    std::size_t arity() const override { return arity_v; }

    std::type_index return_type() const override {
        return std::type_index(return_type_or_void());
    }

    std::vector<std::type_index> argument_types() const override {
        return argument_types_impl(std::make_index_sequence<arity_v>{});
    }

    std::vector<lua_arg_info> lua_argument_info() const override {
        return lua_argument_info_impl(std::make_index_sequence<arity_v>{});
    }

    bool is_member() const override { return traits::is_member; }

private:
    F fn_;

    // typeid(void) is legal, but naming it directly reads oddly; small helper for clarity.
    struct void_tag {};
    using reportable_ret_t = std::conditional_t<std::is_void_v<ret_t>, void_tag, ret_t>;
    static const std::type_info& return_type_or_void() { return typeid(reportable_ret_t); }

    template <std::size_t I>
    using arg_t = remove_cvref_t<std::tuple_element_t<I, args_tuple>>;

    template <std::size_t... I>
    int invoke_lua_impl(lua_State* L, std::index_sequence<I...>) const {
        if constexpr (std::is_void_v<ret_t>) {
            std::invoke(fn_, lua_stack<arg_t<I>>::get(L, static_cast<int>(I) + 1)...);
            return 0;
        } else {
            auto r = std::invoke(fn_, lua_stack<arg_t<I>>::get(L, static_cast<int>(I) + 1)...);
            lua_stack<ret_t>::push(L, r);
            return 1;
        }
    }

    template <std::size_t... I>
    std::any invoke_any_impl(const std::vector<std::any>& args, std::index_sequence<I...>) const {
        if constexpr (std::is_void_v<ret_t>) {
            std::invoke(fn_, std::any_cast<arg_t<I>>(args[I])...);
            return {};
        } else {
            return std::make_any<ret_t>(
                std::invoke(fn_, std::any_cast<arg_t<I>>(args[I])...)
            );
        }
    }

    template <std::size_t... I>
    std::vector<std::type_index> argument_types_impl(std::index_sequence<I...>) const {
        return { std::type_index(typeid(arg_t<I>))... };
    }

    template <std::size_t... I>
    std::vector<lua_arg_info> lua_argument_info_impl(std::index_sequence<I...>) const {
        return {
            lua_arg_info{
                std::type_index(typeid(arg_t<I>)),
                lua_stack<arg_t<I>>::kind,
                lua_stack<arg_t<I>>::kind == lua_kind::userdata
                    ? std::string(lua_stack<arg_t<I>>::metatable_name())
                    : std::string{}
            }...
        };
    }
};

// Wraps any callable — function pointer, member pointer, or lambda/functor
// (captures allowed) — into a shared, type-erased any_function.
//
// Usage:
//   make_function(&free_fn)                 // free function
//   make_function(&Foo::bar)                 // member function
//   make_function([](Foo* self, int x) {...}) // lambda, incl. capturing ones
template <typename F>
inline std::shared_ptr<any_function> make_function(F&& fn) {
    using DF = std::decay_t<F>;
    return std::make_shared<function_wrapper<DF>>(std::forward<F>(fn));
}

} // namespace leasy::ul2
