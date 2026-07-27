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

#include <string>
#include <chrono>

#include "baseui.h"
#include "bitmap.h"
#include "drawable.h"
#include "../lio.hpp"
#include "../leasy.hpp"
#include "../ily3/basetypes.hpp"
#include "../ily3/math.hpp"

namespace leasy::ui3 {
  using clock = std::chrono::high_resolution_clock;
  struct GraphicalString {
    std::string       text;
    Color             color;
    unsigned int      lifetime; // In seconds!
    clock::time_point point;
    int               linealpha;
  };

  class GraphicalConsole : public Drawable {
  private:
    std::vector<GraphicalString> queue;
    ily3::twin<int> start;
    int last_y;

    inline void drawitactually(const GraphicalString &string, Bitmap &map) {
      auto rect = Text::GetSize((*Font::Default()), string.text);
      map.FillRect(Rect(0, last_y, DisplayUi->GetWidth(), rect.height), Color(0, 0, 0, string.linealpha));
      map.TextDraw(start.x, last_y, string.color, string.text);
      last_y += rect.height;
    }

  public:
    inline GraphicalConsole(const ily3::twin<int> &st) 
      : Drawable(0x1000), start(st) {}
    // FIXME: (please!) shall i change Z_t ? cuz ugh.

    inline void gwrite(const std::string &text, unsigned int lifetime = 3, const Color &color = Color(0, 0, 0xFF, 0xFF)) {
      queue.emplace_back(GraphicalString{
        .text = text,
        .color = color,
        .lifetime = lifetime,
        .point = clock::now(),
        .linealpha = 0xAA,
      });
    }
    
    inline void Draw(Bitmap &map) override {
      auto now = clock::now();
      last_y = start.y;
      std::vector<GraphicalString> preceders; // Candidates for the next draw()!
      
      for (auto &string: queue) {
        if (now - string.point < std::chrono::seconds(string.lifetime)) { // We have time to draw it!
          drawitactually(string, map);
          preceders.push_back(string);
        } else if (now - string.point < std::chrono::seconds(string.lifetime + 1)) { // so there, text died, so we'll shade it out!
          auto alpha = ily3::clamp(0x00, 0xFF, string.color.alpha - 20);
          string.color = Color(string.color.red, string.color.green, string.color.blue, alpha);
          string.linealpha = ily3::clamp(alpha + 60, 0xAA, string.linealpha - 1);
          drawitactually(string, map);
          preceders.push_back(string);
        }
      }

      this->queue.swap(preceders);
    }
  };

  class gui_sink final : public ios::sink, public GraphicalConsole {
  public:
    inline gui_sink(const ily3::twin<int>& st)
      : GraphicalConsole(st) {
      // Self-subscribe haha
      leasy::draw.connect([this](Bitmap* map) {
        this->Draw(*map);
      });
    }

    // TODO?: Make it draw on another screen but man am lazy asf.

    inline void write(std::string_view text) override {
      GraphicalConsole::gwrite(std::string(text));
    }
  };
}

