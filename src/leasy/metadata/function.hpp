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
#include <vector>
#include <typeindex>
#include <functional>
#include <type_traits>

#include "../iky7/nameof.hpp"
#include "../iky7/anyf.hpp"
#include "../kits/rtt.hpp"
#include "../kits/variant.hpp"
#include "../kits/select.hpp"
#include "../ul2/state.hpp"
#include "../ul2/function_traits.hpp"
#include "../lio.hpp"

#include "metadata.hpp"

namespace leasy::metadata {

  class function_base_t : public Data {
  public:
    std::string name;

    inline virtual std::pair<bool, std::string> is_callable(const std::vector<std::any>&) const {
      return {false, "You're trying to call a default-virtual function !"}; 
    }

    inline virtual std::any call(const std::vector<std::any>&) const {
      return {}; // I prefer an API that fails over virtual methods rather than making weird pure virtual things.
    }

    inline virtual std::function<int(lua_State*)> lua() const {
      return [](lua_State*) -> int {
        // And making warnings is the best way to debug!
        io.Warning.writeln(__func__, ": unimplemented binding was called on base ", nameof<decltype(*this)>());
        return 0;
      };
    }
  };

  class function final : public function_base_t {
  private:
    std::function<std::any(const std::vector<std::any>&)> native_bridge;
    std::function<int(lua_State*)> lua_bridge;

  public:
    std::vector<TypeInfo> arguments;
    TypeInfo return_type;

    inline std::pair<bool, std::string> is_callable(const std::vector<std::any> &args) const override {
      return kits::is_callable_with(this->arguments, args);
    }

    inline std::any call(const std::vector<std::any> &args) const override {
      return this->native_bridge(args);
    }

    inline std::function<int(lua_State*)> lua() const override {
      return this->lua_bridge;
    }

    inline void bind(ul2::lstate &state) const override {
      state.bind2(this->name, this->lua_bridge);
    }

    inline Object dump() const override {
      return Map()
             .add("name", name)
             .add("arguments", kits::select(arguments, [](TypeInfo info) { return info.dump(); }))
             .add("return", return_type.dump());
    }

    inline function() : arguments({}), return_type(typeidof<void>()) {
      this->native_bridge = [](const std::vector<std::any>&) -> std::any { return {}; };
      this->lua_bridge = [](lua_State*) -> int { return 0; };
    }

    template <typename Fn>
    inline function(const std::string &name, const Fn &f) {
      this->name = name;
      using traits = ul2::function_traits<Fn>;
      this->native_bridge = iky7::bridgefunc<Fn>(f);
      this->lua_bridge = ul2::lstate::bridge<Fn>(f);
      this->arguments = kits::tuple_types<typename traits::args_tuple>();
      this->return_type = typeidof<typename traits::return_type>();
    }
  };

  class overload_set final : public function_base_t {
  private:
    std::vector<function> funcs;
    std::function<int(lua_State*)> bridge;

  public:

    inline std::pair<bool, std::string> is_callable(const std::vector<std::any> &args) const override {
      size_t i = 0;
      size_t end = funcs.size();

      while (i++ < end) {
        auto r = funcs[i-1].is_callable(args);
        if (r.first) return r;
      }

      return {false, "No such overload found"};
    }

    inline std::any call(const std::vector<std::any> &args) const override {
      size_t i = 0;
      size_t end = funcs.size();

      while (i++ < end) {
        auto r = funcs[i-1].is_callable(args);
        if (r.first) return funcs[i-1].call(args);
      }

      throw std::runtime_error("overload-resolver error: No such overload found when calling " + name + "!");
    }

    inline std::function<int(lua_State*)> lua() const override {
      // That's the fucking point where i need a pure-native value.
      // The fucking issue's that we can't both template a type and serialize it easily
      // in then. So, the trick is playing with C++'s variables: We build the lua-bridge
      // at construction.
      // That's **coding.**
      return this->bridge;
    }

    inline overload_set() {} // bruh.
    
    template <typename... Fs> // why does standard conventions put 's' in plural templated-names ? Tf.
    inline overload_set(const std::string &name, Fs&& ...fs) {
      this->name = name;
      this->bridge = ul2::make_overload(std::forward<Fs>(fs)...);
      // Now, let's bridge native overloads. MUCH more easier.
      (this->funcs.emplace_back(function(name, std::forward<Fs>(fs))), ...);
    }

    inline Object dump() const override {
      return Map()  // Programming's magic.
             .add("name", this->name)
             .add("overloads", kits::select(this->funcs, [](function func) { return func.dump(); }));
    }

    inline void bind(ul2::lstate &state) const override {
      state.bind2(this->name, this->bridge);
    }
  };

  class NativeCallable : public function_base_t {
  private:
    std::shared_ptr<function_base_t> impl;

  public:
    NativeCallable() = default;

    NativeCallable(function f)
      : impl(std::make_shared<function>(std::move(f))) {}

    NativeCallable(overload_set o)
      : impl(std::make_shared<overload_set>(std::move(o))) {}

    function_base_t* operator->() {
      return impl.get();
    }

    const function_base_t* operator->() const {
      return impl.get();
    }

    function_base_t& operator*() {
      return *impl;
    }

    const function_base_t& operator*() const {
      return *impl;
    }

    explicit operator bool() const {
      return static_cast<bool>(impl);
    }

    inline std::pair<bool, std::string> is_callable(const std::vector<std::any>& args) const override {
      return impl->is_callable(args);
    }

    inline std::any call(const std::vector<std::any>& args) const override {
      return impl->call(args);
    }

    inline std::function<int(lua_State*)> lua() const override {
      return impl->lua();
    }

    inline void bind(ul2::lstate& state) const override {
      impl->bind(state);
    }

    inline Object dump() const override {
      return impl->dump();
    }
  };

  template <typename... Fs>
  NativeCallable make_function(const std::string &name, Fs&&... fs) {
    if constexpr (sizeof...(fs) == 0) {
      return function(name, []() {}); // That one does cleary something. LOL.
    } else if constexpr (sizeof...(fs) == 1) {
      return function(name, std::forward<Fs>(fs)...);
    } else {
      return overload_set(name, std::forward<Fs>(fs)...);
    }
  }
}

