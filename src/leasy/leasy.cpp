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
#include "metadata/glues/lua_glues.hpp"
#include "meta/node/node.hpp"

#include "game_map.h"
#include "spriteset_map.h"

namespace leasy {
  meta2::node::Meta2Context meta2::node::meta2Context {std::make_shared<Node>()};

  namespace ily3 {
    extern std::vector<std::shared_ptr<Drawable>> leasy_draw_queue;
  }

  static void leasy_secondary_tests() {
    auto id = Game_Map::GetMapId();
    auto highest = Game_Map::GetHighestEventId();
    auto nid = Game_Map::GetNextAvailableEventId();
    auto clone = Game_Map::CloneMapEvent(id, highest, 90, 90, nid, "Bob");
    //Spriteset_Map::CreateSprite;
    auto event = Game_Map::GetEvent(nid);
    auto chara = Game_Event::GetCharacter(Game_Character::CharThisEvent, nid);

    if (! clone) {
      throw std::runtime_error("BOB AIN'T ALIVE!");
    }
  }

  namespace app {
    static bool should_exit = false;
    static auto last = std::chrono::high_resolution_clock::now();
    static bool leasy_enabled = true;
    static auto infoGuiSink = std::make_shared<ui3::gui_sink>(ily3::make_twin<int>(0, 0));
    static auto r = io().Info.attach(infoGuiSink);

    bool exit_requested() {
      return should_exit;
    }

    void request_exit() {
      should_exit = true;
    }

    void ready(void) {
      if (! leasy_enabled) return;
      engine::NativeEvents::onReady.call();

      io().Info.writeln("binding EasyRPGPlayer!");
      metadata::AppDomain().bind(ily3::global::state);
      ily3::global::state.call<void>("leasy.User.ready");
      io().Info.writeln("reflection metadata size: ", kits::format_bytes(metadata::AppDomain().getMetadataSize()));
      meta2::node::meta2Context.ready();

      engine::NativeEvents::onMapLoaded.addCallback([](const String& miam) {
        io().Debug.writeln(miam);
        if (miam == "Map0003.lmu") {
          leasy_secondary_tests();
        }
      });
    }
    
    void process() {
      if (! leasy_enabled) return;
      auto now = std::chrono::high_resolution_clock::now();
      long double delta = std::chrono::duration<long double>(now - last).count();

      engine::NativeEvents::onProcess.call(delta);
      ily3::global::state.call<void>("leasy.User.process", delta);
      meta2::node::meta2Context.update(delta);
    }

    void draw(Bitmap *map) {
      if (! leasy_enabled) return;

      engine::NativeEvents::onDraw.call(map);
      ily3::global::state.call<void>("leasy.User.draw");
      
      for (const auto &drawable: ily3::leasy_draw_queue) {
        drawable->Draw(*map);
      }

      ui3::update_windows();
      meta2::node::meta2Context.draw(map);
    }

    void exit(void) {
      engine::NativeEvents::onExit.call();
    }

    void disable() {
      io().Info.writeln(">>> leasy is disabled ! (You'll need to reboot the engine in order to enable it!)");
      leasy_enabled = false;
    }

    bool is_enabled() {
      return leasy_enabled;
    }
  }
}
