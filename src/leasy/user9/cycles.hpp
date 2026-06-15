#pragma once

#include <string>
#include <memory>
#include <functional>

#include "bitmap.h"

namespace leasy::user9 {
  std::shared_ptr<std::function<void(double)>> onupdate(const std::function<void(double)>&);
  std::shared_ptr<std::function<void(Bitmap&)>> ondraw(const std::function<void(Bitmap&)>&);

  void nextupdate(const std::function<void(double)>&);
  void nextdraw(const std::function<void(Bitmap&)>&);

  void onexit(const std::function<void(void)>&);
}