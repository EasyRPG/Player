#pragma once

#include <string>
#include "../drawable_object.hpp"

namespace leasy::libs2 {
  class Text : public DrawableObject {
  public:
    std::string text;
    Color       color;

    inline void Draw(Bitmap &map) override {
      map.TextDraw(this->get_rect(), this->color, this->text);
    }

    inline Text() {}

    inline Text(const std::string &str, const Color &color, const ily3::twin<int> &pos, const ily3::twin<int> &size) {
      this->text = str;
      this->color = color;
      this->pos = pos;
      this->siz = size;
    }
  };
}