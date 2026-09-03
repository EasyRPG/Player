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
#include "game_message.h"
#include "../lio.hpp"
#include "../leasy.hpp"
#include "../ily3/basetypes.hpp"
#include "../ily3/math.hpp"

namespace leasy::ui3 {
  inline String removeAnsiEscapes(const String &input) {
    String output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size();) {
      if (input[i] == '\x1B' &&
          i + 1 < input.size() &&
          input[i + 1] == '[') {
        i += 2;

        while (i < input.size() &&
               !(input[i] >= 0x40 && input[i] <= 0x7E)) {
          ++i;
        }

        if (i < input.size()) ++i;

        continue;
      }

      if (input[i] == '\x1B' && i + 1 < input.size()) {
        i += 2;
        continue;
      }

      output += input[i++];
    }

    return output;
  }

  using clock = std::chrono::high_resolution_clock;

  struct GraphicalString {
    String text;
    Color color;
    size_t height;
    unsigned int lifetime; // In seconds!
    clock::time_point point;
    int lineAlpha;
  };

  class GraphicalConsole : public Drawable {
  private:
    std::vector<GraphicalString> queue;
    ily3::twin<int> start;
    int last_y{};

    void drawItActually(const GraphicalString &string, Bitmap &map) {
      auto rect = Text::GetSize(*Font::Default(), string.text);

      // That's the black-transparent thingy underneath the text (idk hows called)
      map.FillRect(Rect(0, last_y, DisplayUi->GetWidth(), string.height), Color(0, 0, 0, string.lineAlpha));

      map.TextDraw(start.x, last_y, string.color, string.text);
      last_y += string.height;
    }

  public:
    GraphicalConsole(const ily3::twin<int> &st): Drawable(0x1000), start(st) {}

    // FIXME: (please!) shall i change Z_t ? cuz ugh.

    void gWrite(const String &text, unsigned int lifetime = 6, const Color &color = Color(0x02, 230, 150, 0xFF)) {
      auto getGlyphSize = [](char c) -> size_t {
        return Text::GetSize(*Font::Default(), String::from(c)).width;
      };

      auto modified = text.fitToWidth(DisplayUi->GetWidth(), getGlyphSize);

      queue.emplace_back(GraphicalString{
        .text = modified,
        .color = color,
        .height = modified.countOf('\n') * Text::GetSize(*Font::Default(), "A").height,
        .lifetime = lifetime,
        .point = clock::now(),
        .lineAlpha = 0xEE,
      });
    }

    void Draw(Bitmap &map) override {
      const auto now = clock::now();
      last_y = start.y;
      std::vector<GraphicalString> preceders;

      for (const auto &string : queue) {
        const auto elapsed = now - string.point;
        const auto lifetime = std::chrono::seconds(string.lifetime);
        constexpr auto fadeDuration = std::chrono::seconds(1);

        if (elapsed < lifetime) {
          drawItActually(string, map);
          preceders.push_back(string);
          continue;
        }

        const auto fadeElapsed = elapsed - lifetime;

        if (fadeElapsed < fadeDuration) {
          const float t = std::chrono::duration<float>(fadeElapsed).count();
          const int alpha = static_cast<int>(string.color.alpha * (1.0f - t));
          const int lineAlpha = static_cast<int>(string.lineAlpha * (1.0f - t));
          GraphicalString faded = string;

          faded.color = Color(
            string.color.red,
            string.color.green,
            string.color.blue,
            ily3::clamp(0, 255, alpha)
          );

          faded.lineAlpha = ily3::clamp(0, 255, lineAlpha);
          drawItActually(faded, map);
          preceders.push_back(string);
        }
      }

      queue.swap(preceders);
    }
  };

  class gui_sink final : public ios::sink, public GraphicalConsole {
  public:
    gui_sink(const ily3::twin<int> &startRect) : GraphicalConsole(startRect) {
      // Self-subscribe haha
      engine::NativeEvents::onDraw.addCallback([this](Bitmap *map) {
        this->Draw(*map);
      });
    }

    // FIXME: Make it draw on another screen but man am lazy asf.
    // FIXME: Make longer messages on multiple lines
    // TODO: Implement this in Lua bindings!
    // TODO: expose io() to Lua!

    void write(std::string_view text) override {
      gWrite(removeAnsiEscapes(ToString(text)));
    }
  };
}