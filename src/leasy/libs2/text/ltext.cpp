#include "text.hpp"
#include "../../ily3/libsys.hpp"
#include "../../ily3/basetypes.hpp"
#include "../../ily3/drawqueue.hpp"

namespace {
  using namespace leasy;

  void drawtext(const std::string &txt, uint32_t color, int x, int y, int width, int height) {
    ily3::drawqueue(std::make_shared<libs2::Text>(txt, ily3::color_from_u32(color), ily3::make_twin<int>(x, y), ily3::make_twin<int>(width, height)));
  }

  ily3::lib ltext = {
    .name = "leasy.text",
    .funcs = {
      ily3::make_fun<drawtext>("write"),
    }
  };

  bool OK = ([](void) -> bool {
    ily3::addlib(ltext);
    return false;
  })();
}
