#pragma once

#include <filesystem>
#include "drawable.h"
#include "image_png.h"
#include "../../ldebug.hpp"
#include "../../ily3/memguard.hpp"
#include "../../ily3/basetypes.hpp"

namespace leasy::libs2 {
  class Image : public Drawable {
  protected:
    std::unique_ptr<Bitmap> map;
    std::filesystem::path   path;
    ily3::guard<ImageOut>   imgout; /* As i see, EasyRPG doesn't free resources !*/
    ily3::twin<int>         pos;
    Opacity                 opa;

  public:
    enum Type {
      PNG, BMP, XYZ,
    };

    /** @brief creates an empty image. */
    Image();

    /** @brief creates an image and loads the image resource at the given path. */
    Image(const std::filesystem::path&/* path */, const Type&/* type */);

    void Open(const std::filesystem::path&/* path */, const Type&/* type */);
    void Destroy();
    void Draw(Bitmap&) override;

    ImageOut *SourceImage();
    std::filesystem::path FilePath();
    ily3::twin<int> &Position();
    ily3::twin<int> Size();
    Opacity *Opacity();
  };
}