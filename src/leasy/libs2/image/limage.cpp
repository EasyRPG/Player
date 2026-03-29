#include "../../ily3/libsys.hpp"
#include "../../ily3/basetypes.hpp"
#include "../../ily3/drawqueue.hpp"
#include "image.hpp"

#include <string>
#include <filesystem>
#include <unordered_map>

namespace /* nameless HAHAHAHA */ {
  using namespace leasy;
  static unsigned int idref = 0; /* whut */
  std::unordered_map<std::string, unsigned int> cache;
  std::unordered_map<unsigned int, std::shared_ptr<libs2::Image>> images;

  libs2::Image::Type miav(const std::filesystem::path &p) {
    auto e = p.extension(); // IMAGINE UR NO CAT

    if (e == ".png") return libs2::Image::PNG;
    else if (e == ".jpg") return libs2::Image::XYZ;
    else return libs2::Image::BMP;
  }

  unsigned int imgnew(const char *path) {
    auto p = std::filesystem::path(path).lexically_normal();

    if (cache.find(p.string()) != cache.end()) {
      // if its cached guys why loading it again please ...
      return cache[p.string()]; // this cute
    } else {
      // stfu i like else blocks.
      auto cid = idref++;
      images[cid] = std::make_shared<libs2::Image>(p, miav(p));
      return cid;
    }
  };

  void imgdraw(unsigned int id, int x, int y) {
    if (images.find(id) == images.end()) {
      ulthrow("invalid id: " + std::to_string(id));
    }

    auto img = (images[id]);
    auto pos = img->position();
    *pos = ily3::make_twin<int>(x, y);
    ily3::drawqueue(img);
  }

  void imgdelete(unsigned int id) {
    if (images.find(id) != images.end()) {
      images[id].reset();
    }
  }

  ily3::lib imglib = {
    .name = "leasy.image",
    .funcs = {
      ily3::make_fun<imgnew>("new"),
      ily3::make_fun<imgdraw>("draw"),
      ily3::make_fun<imgdelete>("delete"),
    }
  };

  bool OK = ([]() -> bool {
    ily3::addlib(imglib);
    return true;
  })();
}

