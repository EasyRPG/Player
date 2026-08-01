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

#include <functional>
#include <vector>

#include "kits/preload.hpp"

namespace leasy {
  template<typename... Args>
  class Signal {
  public:
    using Callback = std::function<void(Args...)>;

    kits::preload_result connect(Callback cb) {
      callbacks.push_back([cb = std::move(cb)](Args... args) {
        std::puts("wtf?");
        cb(args...);
      });
      return {};
    }

    void emit(Args... args) {
      for (auto& cb : callbacks) {
        cb(args...);
      }
    }

  private:
    std::vector<Callback> callbacks;
  };
}