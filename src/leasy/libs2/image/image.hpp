#pragma once

#include <filesystem>
#include "drawable.h"
#include "image_png.h"
#include "../drawable_object.hpp"
#include "../../ldebug.hpp"
#include "../../ily3/memguard.hpp"
#include "../../ily3/basetypes.hpp"

namespace leasy::libs2 {
  class Image : public DrawableObject {
  protected:
    std::shared_ptr<Bitmap> map;
    std::filesystem::path   path;
    ily3::guard<ImageOut>   imgout; /* As i see, EasyRPG doesn't free resources !*/

  public:
    enum Type {
      PNG, BMP, XYZ,
    };

    /** @brief creates an empty image. */
    Image();

    /** @brief creates an image and loads the image resource at the given path. */
    Image(const std::filesystem::path&/* path */, const Type&/* type */);

    void open(const std::filesystem::path&/* path */, const Type&/* type */);
    void destroy();
    void Draw(Bitmap&) override;

    ImageOut *image();
    std::filesystem::path filepath();
  };
}