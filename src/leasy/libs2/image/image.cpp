#include "image.hpp"
#include "../../ily3/easyfs.hpp"
#include "../../ul2/ulexception2.hpp"
#include "../../lio.hpp"

#include "image_bmp.h"
#include "image_png.h"
#include "image_xyz.h"

#include <iostream>

namespace leasy::libs2 {

  static Image::Type DetectType(const std::filesystem::path& path) {
    auto ext = path.extension().string();

    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".png") return Image::PNG;
    if (ext == ".bmp") return Image::BMP;
    if (ext == ".xyz") return Image::XYZ;

    return Image::PNG;
  }

  void Image::open(const std::filesystem::path &path, const Type &hint) {
    this->destroy();

    this->imgout = ily3::guard<ImageOut>();
    this->path = path;

    auto strname = path.lexically_normal().string();
    io.Debug.writeln("img> loading image: ", strname);
    
    auto stream = ily3::efs::nfs.OpenInputStream(strname);
    
    if (!stream) {
      ulthrow("Failed to open image stream: " + strname);
    }
    
    Type actual = DetectType(path);
    
    if (hint != actual) {
      io.Debug.writeln("<img> using hint over detected type", strname);
      actual = hint;
    }

    bool success = true;

    try {
      switch (actual) {
        case PNG:
          ImagePNG::Read(stream, true, this->imgout.resource);
          break;

        case BMP:
          ImageBMP::Read(stream, true, this->imgout.resource);
          break;

        case XYZ:
          ImageXYZ::Read(stream, true, this->imgout.resource);
          break;

        default:
          success = false;
          break;
      }
    } catch (...) {
      success = false;
    }

    if (!success || !this->imgout.resource.pixels) {
      ulthrow("Image loading failed or invalid format: " + strname);
    }

    this->imgout.deleter = [](ImageOut *img) -> void {
      if (img && img->pixels) {
        free(img->pixels);
        img->pixels = nullptr;
      }
    };

    this->map = std::make_shared<Bitmap>(
      this->imgout.resource.pixels,
      this->imgout.resource.width,
      this->imgout.resource.height,
      0,
      Bitmap::opaque_image_format
    );
  }

  Image::Image() {}

  Image::Image(const std::filesystem::path &path, const Type &type) {
    this->open(path, type);
  }

  void Image::destroy() {
    this->map.reset();
    this->imgout.reset();
  }

  ImageOut *Image::image() {
    return &this->imgout.resource;
  }

  std::filesystem::path Image::filepath() {
    return this->path;
  }

  void Image::Draw(Bitmap &dst) {
    if (!this->map) return; // prevent crash (lol)

    dst.Blit(
      this->pos.x,
      this->pos.y,
      (*this->map),
      this->map->GetRect(),
      this->opa
    );
  }

}