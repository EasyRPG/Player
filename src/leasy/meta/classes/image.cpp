//
// Created by @wys on 12/08/2026.
//

#include <fstream>
#include "image.hpp"

#include "image_bmp.h"
#include "image_png.h"
#include "image_xyz.h"
#include "leasy/types/string.hpp"

namespace leasy::meta2::classes {
  std::pair<bool, ImageOut> loadImageRaw(const std::vector<char> &rawdata, IMAGE_FILE_KIND kind) {
    ImageOut out{};
    switch (kind) {
      case XYZ: return {ImageXYZ::Read((uint8_t*)rawdata.data(), rawdata.size(), true, out), out};
      case PNG: return {ImagePNG::Read((void*)rawdata.data(), true, out), out};
      case BMP: return {ImageBMP::Read((uint8_t*)rawdata.data(), rawdata.size(), true, out), out};
      default: return {
        false, out
      };
    }
  }

  static IMAGE_FILE_KIND getFileKind(const String &string) {
    if (string.endsWith(".png")) {
      return IMAGE_FILE_KIND::PNG;
    }
    if (string.endsWith(".xyz")) {
      return IMAGE_FILE_KIND::XYZ;
    }
    if (string.endsWith(".bmp")) {
      return IMAGE_FILE_KIND::BMP;
    }

    return IMAGE_FILE_KIND::UNKNOWN;
  }

  Image::Image() = default;

  Image::Image(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (! file) throw std::runtime_error(__func__ + std::string(": can't open file " + path.string()));

    const std::istreambuf_iterator<char> it{file}, end;
    std::vector<char> buff {it, end};
    auto result = loadImageRaw(buff, getFileKind(path.string()));
    if (! result.first) throw std::runtime_error("unable to load any image from " + path.string());
    this->data = result.second;
    this->_owned = false;
  }

  Image::Image(void *bytes, int w, int h, int b) {
    this->data.bpp = b;
    this->data.width = w;
    this->data.height = h;
    this->data.pixels = bytes;
    this->_owned = false;
  }

  Image::Image(const ImageOut &out) {
    this->data = out;
    this->_owned = false;
  }

  Image::~Image() {
    if (this->_owned) {
      free(this->data.pixels);
    }
  }

  Image Image::duplicate() const {
    throw std::runtime_error("not implemented!");
  }

  ImageOut Image::EZRPG() const {
    return this->data;
  }

  void* Image::pixels() const {
    return data.pixels;
  }

  int Image::width() const {
    return data.width;
  }

  int Image::height() const {
    return data.height;
  }

  int Image::bpp() const {
    return data.bpp;
  }


}
