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
#include <string>

#include "ily3/ily3.hpp"
#include "lio.hpp"

class Bitmap;

namespace leasy::engine {
  template <class...Args>
  void event(const std::string &name, Args...args) {
    std::string rname = "leasy.Engine." + name;
    try {
      ily3::global::state.call<void>(rname, args...);
    } catch (const ul2::ulexception2 &e) {
      io().Warning.writeln("error (ul2) during event invocation: ", e.whut());
      // Sometimes, it's just because the callback does not exist!
      // So let's just ensure it haha
      ily3::global::state.dostring(rname + " = " + rname + " or function()end");
    } catch (const std::exception &e) {
      io().Warning.writeln("error (stdc++) during event invocation: ", e.what());
    }
  }

  namespace NativeEvents {
    template <typename... Args>
    class Event {
    public:
      using Callback = std::function<void(Args...)>;

      void call(Args... args) {
        for (auto& cb : callbacks) {
          if (cb && *cb) {
            (*cb)(args...);
          }
        }

        callbacks.erase(
          std::remove_if(callbacks.begin(), callbacks.end(),
            [](const auto& cb) {
              return !cb || !*cb;
            }),
          callbacks.end()
        );
      }

      std::shared_ptr<Callback> addCallback() {
        callbacks.push_back(std::make_shared<Callback>());
        return callbacks.back();
      }

      template <typename F>
      std::shared_ptr<Callback> addCallback(F fun) {
        callbacks.push_back(std::make_shared<Callback>(fun));
        return callbacks.back();
      }

    private:
      std::vector<std::shared_ptr<Callback>> callbacks;
    };

    template <typename... Args>
    constexpr auto makeInlineEvent() {
      return [] {
        static Event<Args...> event{};
        return event;
      };
    }

    extern Event<> onMapInit;
    extern Event<std::string> onMapLoaded;
    extern Event<> onMapQuit;
    extern Event<long double> onProcess;
    extern Event<> onReady;
    extern Event<Bitmap*> onDraw;
    extern Event<> onExit;
  }
}
