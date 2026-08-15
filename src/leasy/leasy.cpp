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

#include <chrono>
#include <string>
#include <fstream>

#include "events.hpp"
#include "ul2/state.hpp"
#include "ily3/ily3.hpp"
#include "ui/window.hpp"
#include "ui/winstream.hpp"
#include "metadata/Domain.hpp"
#include "signals.hpp"
#include "metadata/glues/lua_glues.hpp"
#include "meta/node/node.hpp"

namespace leasy {
  meta2::node::Meta2Context meta2::node::meta2Context {std::make_shared<meta2::node::Node>()};

  namespace ily3 {
    extern std::vector<std::shared_ptr<Drawable>> leasy_draw_queue;
  }

  Signal<> ready   = {};
  Signal<double> process = {};
  Signal<Bitmap*> draw = {};



  namespace app {
    static bool should_exit = false;
    static auto last = std::chrono::high_resolution_clock::now();
    static bool leasy_enabled = true;

    bool exit_requested() {
      return should_exit;
    }

    void request_exit() {
      should_exit = true;
    }

    void ready(void) {
      if (! leasy_enabled) return;
      leasy::ready.emit();
      io().Debug.writeln("binding EasyRPGPlayer!");
      metadata::AppDomain().bind(ily3::global::state);
      ily3::global::state.call<void>("leasy.User.ready");
      io().System.writeln("reflection metadata size: ", kits::format_bytes(metadata::AppDomain().getMetadataSize()));
      meta2::node::meta2Context.ready();
    }
    
    void process() {
      if (! leasy_enabled) return;
      auto now = std::chrono::high_resolution_clock::now();
      double delta = std::chrono::duration<double>(now - last).count();

      leasy::process.emit(delta);
      ily3::global::state.call<void>("leasy.User.process", delta);
      meta2::node::meta2Context.update(delta);
    }

    void draw(Bitmap *map) {
      if (! leasy_enabled) return;

      leasy::draw.emit(map);
      ily3::global::state.call<void>("leasy.User.draw");
      
      for (const auto &drawable: ily3::leasy_draw_queue) {
        drawable->Draw(*map);
      }

      ui3::update_windows();
      meta2::node::meta2Context.draw(map);
    }

    void exit(void) {}

    void disable() {
      io().System.writeln(">>> leasy is disabled ! (You'll need to reboot the engine in order to enable it!)");
      leasy_enabled = false;
    }

    bool is_enabled() {
      return leasy_enabled;
    }
  }
}
