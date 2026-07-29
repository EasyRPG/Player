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

#warning TODO: leasy.errors[] ~ classes that make errors better. (can unpromote them as warnings etc)
#warning TODO: leasy.settings?
#warning TODO: leasy.globals?

#include <chrono>
#include <string>
#include <fstream>
#include <string_view>

#include "events.hpp"
#include "ldebug.hpp"
#include "ul2/state.hpp"
#include "ily3/ily3.hpp"
#include "diag5/here.h"

#include "ui/window.hpp"
#include "ui/winstream.hpp"

#include "metadata/namespace.hpp"
#include "metadata/json.hpp"

#include "signals.hpp"

namespace leasy {
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
      ily3::global::state.call<void>("leasy.User.ready");
      std::ofstream of("dump.json");
      metadata::json::write(of, metadata::EasyRPG().dump());
    }
    
    void process() {
      if (! leasy_enabled) return;
      auto now = std::chrono::high_resolution_clock::now();
      double delta = std::chrono::duration<double>(now - last).count();

      leasy::process.emit(delta);
      ily3::global::state.call<void>("leasy.User.process", delta);      
    }

    void draw(Bitmap *map) {
      if (! leasy_enabled) return;

      leasy::draw.emit(map);
      ily3::global::state.call<void>("leasy.User.draw");
      
      for (const auto &drawable: ily3::leasy_draw_queue) {
        drawable->Draw(*map);
      }

      ui3::update_windows();
    }

    void exit(void) { }

    void disable() {
      io().System.writeln(">>> leasy is disabled ! (You'll need to reboot the engine in order to enable it!)");
      leasy_enabled = false;
    }

    bool is_enabled() {
      return leasy_enabled;
    }
  }
}
