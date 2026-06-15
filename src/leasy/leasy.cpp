#include <chrono>
#include <string>
#include <fstream>
#include <string_view>

#include "events.hpp"
#include "ldebug.hpp"
#include "ul2/state.hpp"
#include "ily3/ily3.hpp"
#include "libs2/image/image.hpp"
#include "diag5/here.h"

#include "ui/window.hpp"
#include "ui/winstream.hpp"

namespace leasy {
  namespace ily3 {
    extern std::vector<std::shared_ptr<Drawable>> leasy_draw_queue;
  }

  namespace user9::share {
    extern std::unordered_map<unsigned long long, std::weak_ptr<std::function<void(double)>>> holdupdates;
    extern std::unordered_map<unsigned long long, std::weak_ptr<std::function<void(void)>>> holddraws;
    extern std::vector<std::function<void(void)>> atexitholds;
    extern std::vector<std::function<void(double)>> holdnextupdate;
    extern std::vector<std::function<void(void)>> holdnextdraw;
  }

  namespace app {
    static bool should_exit = false;
    static auto last = std::chrono::high_resolution_clock::now();
    static bool leasy_enabled = true;
    ui3::GraphicalConsole console = ui3::GraphicalConsole(ily3::make_twin<int>(0, 5));

    bool exit_requested() {
      return should_exit;
    }

    void request_exit() {
      should_exit = true;
    }

    void ready(void) {
      if (! leasy_enabled) return;
      // Use a chrono + ... so i can bench start-time ?
      // Still ain't done

      ily3::global::state.call<void>("leasy.User.ready");
    }
    
    void process() {
      if (! leasy_enabled) return;
      auto now = std::chrono::high_resolution_clock::now();
      double delta = std::chrono::duration<double>(now - last).count();

      ily3::global::state.call<void>("leasy.User.process", delta);

      std::vector<unsigned long long> ids {};
      for (const auto &[id, func]: user9::share::holdupdates) {
        if (! func.expired()) {
          (*func.lock())(delta);
        } else ids.push_back(id);
      }

      for (const auto &func: user9::share::holdnextupdate) {
        func(delta);
      }

      user9::share::holdnextupdate.clear();
      for (const auto &id: ids) user9::share::holdupdates.erase(id);

      last = now;
    }

    void draw(Bitmap *map) {
      if (! leasy_enabled) return;
      ily3::global::state.call<void>("leasy.User.draw");
      
      for (const auto &drawable: ily3::leasy_draw_queue) {
        drawable->Draw(*map);
      }

      ui3::update_windows();

      std::vector<unsigned long long> ids {};
      for (const auto &[id, func]: user9::share::holddraws) {
        if (! func.expired()) {
          (*func.lock())();
        } else ids.push_back(id);
      }

      for (const auto &func: user9::share::holdnextdraw) {
        func();
      }

      user9::share::holdnextdraw.clear();
      for (const auto &id: ids) user9::share::holddraws.erase(id);

      std::vector<std::shared_ptr<Drawable>> queue;
      queue.reserve(32);
      ily3::leasy_draw_queue.swap(queue);
      console.Draw(*map);
    }

    void exit(void) { }

    void disable() {
      io.System.writeln(">>> leasy is disabled ! (You'll need to reboot the engine in order to enable it!)");
      leasy_enabled = false;
    }

    bool is_enabled() {
      return leasy_enabled;
    }
  }
}
