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
#include <functional>

#include "../iky7/anyf.hpp"
#include "../kits/rtt.hpp"
#include "../kits/variant.hpp"
#include "../kits/select.hpp"
#include "../ul2/state.hpp"
#include "../ul2/function_traits.hpp"
#include "../lio.hpp"

#include "structs/metadata.hpp"
#include "type.hpp"
#include "function_base.hpp"

namespace leasy::metadata {
  class function final : public function_base_t {
  private:
    std::function<std::any(std::vector<std::any>&)> native_bridge;
    std::function<int(lua_State*)> lua_bridge;

  public:
    std::vector<std::shared_ptr<Class>> arguments;
    std::shared_ptr<Class> return_type;

    inline std::vector<FunctionSignature> getSignatures() const override {
      return { { .arguments = arguments, .returnType = return_type} };
    }

    inline std::pair<bool, std::string> is_callable(std::vector<std::any> &args) const override {
      return kits::is_callable_with(this->arguments, args);
    }

    inline std::any call(std::vector<std::any> &args) const override {
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
             .add("arguments", kits::select(arguments, [](const std::shared_ptr<Class> &info) { 
                return info->minimal_dump();
              }))
             .add("return", return_type->minimal_dump());
    }

    inline function() : arguments({}), return_type(typeidof<void>()) {
      this->native_bridge = [](std::vector<std::any>&) -> std::any { return {}; };
      this->lua_bridge = [](lua_State*) -> int { return 0; };
    }

    template <typename Fn>
    inline function(const std::string &name, const Fn &f) {
      this->name = name;
      using traits = ul2::function_traits<Fn>;
      this->native_bridge = iky7::bridgefunc(f);
      this->lua_bridge = ul2::lstate::bridge(f);
      this->arguments = kits::tuple_types<typename traits::args_tuple>();
      this->return_type = typeidof<typename traits::return_type>();
    }

    size_t getMetadataSize() const override {
      return sizeof(*this)
      + sizeof(decltype(arguments)) + sizeof(arguments.capacity()) * sizeof(decltype(arguments)::value_type)
      + sizeof(return_type);
    }
  };

  class overload_set final : public function_base_t {
  private:
    std::vector<function> funcs;
    std::function<int(lua_State*)> bridge;

  public:

    inline std::vector<FunctionSignature> getSignatures() const override {
      return kits::select(funcs, [](const function &fn) {
        return fn.getSignatures()[0];
      });
    }

    inline size_t getMetadataSize() const override {
      auto funcsize{0ull};
      for (auto i{0ull}; i < funcs.size(); i++) funcsize += funcs[i].getMetadataSize();
      return (
        sizeof(*this) + sizeof(funcs) + sizeof(decltype(funcs)::value_type) * funcs.capacity() + sizeof(bridge) + funcsize
      );
    }

    inline std::pair<bool, std::string> is_callable(std::vector<std::any> &args) const override {
      size_t i = 0;
      size_t end = funcs.size();

      while (i++ < end) {
        auto r = funcs[i-1].is_callable(args);
        if (r.first) return r;
      }

      return {false, "No such overload found"};
    }

    inline std::any call(std::vector<std::any> &args) const override {
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

    inline overload_set() = default; // bruh.
    
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
             .add("overloads", kits::select(this->funcs, [](const function& func) { return func.dump(); }));
    }

    inline void bind(ul2::lstate &state) const override {
      state.bind2(this->name, this->bridge);
    }
  };

  template <typename... Fs>
  inline std::shared_ptr<function_base_t> make_function(const std::string &name, Fs&&... fs) {
    if constexpr (sizeof...(fs) == 0) {
      return std::make_shared<function>(function(name, []() {})); // That one does cleary something. LOL.
    } else if constexpr (sizeof...(fs) == 1) {
      return std::make_shared<function>(function(name, std::forward<Fs>(fs)...));
    } else {
      return std::make_shared<overload_set>(overload_set(name, std::forward<Fs>(fs)...));
    }
  }
}
