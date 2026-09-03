//
// Created by @wys on 31/08/2026.
//

#include "events.hpp"

namespace leasy::engine::NativeEvents {
  Event<> onMapInit {};
  Event<std::string> onMapLoaded {};
  Event<> onMapQuit {};
  Event<long double> onProcess {};
  Event<> onReady {};
  Event<Bitmap*> onDraw {};
  Event<> onExit {};
}