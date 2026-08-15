//
// Created by @wys on 14/08/2026.
//

#include "sprite2d.hpp"
#include "leasy/metadata/Domain.hpp"

namespace leasy::meta2::node {
  Sprite2D::Sprite2D() = default;

  Sprite2D::Sprite2D(const std::filesystem::path &p, bool transparent) {
    auto file = new std::filebuf();
    file->open(p, std::ios::in);
    if (! file->is_open()) {
      throw std::runtime_error("Sprite2D: no such file: " + p.string() + "!");
    }

    load(file, transparent);
  }

  Sprite2D::Sprite2D(std::istream *stream, bool transparent) {
    load(stream->rdbuf(), transparent);
  }

  void Sprite2D::draw(Bitmap *map) {
    auto x = this->x;
    auto y = this->y;
    auto rect = this->selfMap->GetRect();
    auto src = (this->selfMap);
    map->Blit(x, y, *src, rect, this->opacity);
    io().Debug.writeln("drawing");
  }

  void Sprite2D::load(std::streambuf *stream, bool transparent) {
    auto fss = Filesystem_Stream::InputStream(stream, "leasy-file-Sprite2D");
    selfMap = std::make_shared<Bitmap>(std::move(fss), transparent, 0);
  }

  namespace {
    bool ok = [] {
      metadata::AppDomain().getAssemblyOrCreate<metadata::BuiltInAssembly>("node")->addType<Sprite2D>(
        metadata::make_class<Sprite2D, Node2D>()
        .method("load", [](Sprite2D &self, std::streambuf *streamptr, bool transparent) {
          return self.load(streamptr, transparent);
        })
        .done()
      );
      return false;
    }();
  }
}
