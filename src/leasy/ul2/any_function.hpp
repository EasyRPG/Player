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

#include <any>
#include <string>
#include <typeindex>
#include <vector>

// Forward declaration only — avoids dragging the Lua C API into every
// translation unit that just wants to do reflection (std::any calls).
struct lua_State;

namespace leasy::ul2 {

// What "shape" of Lua value an argument expects. Used at runtime to pick
// the right overload when a lua_method has several of them, since Lua
// itself carries no static types on its stack.
enum class lua_kind {
    boolean,
    number,
    string,
    userdata
};

struct lua_arg_info {
    std::type_index type;
    lua_kind        kind;
    std::string     metatable_name; // only meaningful when kind == userdata
};

// Type-erased, callable "reflected function". One instance of a
// function_wrapper<F> (see function_wrapper.hpp) implements this for
// any F: free function, member function pointer, or lambda/functor.
struct any_function {
    virtual ~any_function() = default;

    // Reads its arguments directly off the Lua stack (indices 1..arity),
    // pushes the return value (if any), and returns the Lua-convention
    // result count (0 or 1).
    virtual int invoke_lua(lua_State* L) const = 0;

    // Pure C++ calling convention. For member functions, args[0] must be
    // a std::any holding a `C*` (the instance) — every other argument
    // follows in declared order.
    virtual std::any invoke_any(std::vector<std::any>& args) const = 0;

    virtual std::size_t arity() const = 0;
    virtual std::type_index return_type() const = 0;
    virtual std::vector<std::type_index> argument_types() const = 0;
    virtual std::vector<lua_arg_info> lua_argument_info() const = 0;
    virtual bool is_member() const = 0;
};

} // namespace leasy::ul2
