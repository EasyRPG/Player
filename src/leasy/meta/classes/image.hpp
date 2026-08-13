//
// Created by @wys on 12/08/2026.
//

#ifndef EASYRPG_PLAYER_IMAGE_HPP
#define EASYRPG_PLAYER_IMAGE_HPP

#include <memory>
#include <filesystem>

#include "bitmap.h"

namespace leasy::meta2::classes {
  class Image {
    ImageOut data;
    bool _owned{ false };

  public:
    Image();
    Image(const std::filesystem::path&);
    Image(void *bytes, int w, int h, int b);
    Image(const ImageOut&);
    ~Image();

    Image duplicate() const;
    ImageOut EZRPG() const;
    void* pixels() const;
    int width() const;
    int height() const;
    int bpp() const;
  };

  enum IMAGE_FILE_KIND {
    BMP,
    PNG,
    XYZ,
    UNKNOWN,
  };

  std::pair<bool, ImageOut> loadImageRaw(const std::vector<char>&, IMAGE_FILE_KIND);
}

#endif //EASYRPG_PLAYER_IMAGE_HPP
