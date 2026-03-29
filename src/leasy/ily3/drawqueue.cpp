#include "drawable.h"
#include "drawqueue.hpp"
#include "bitmap.h"
#include <vector>
#include <functional>

#include "../ldebug.hpp"

namespace leasy::ily3 {
  std::vector<std::shared_ptr<Drawable>> leasy_draw_queue;

  void drawqueue(const std::shared_ptr<Drawable> &drawable) {
    leasy_draw_queue.push_back(drawable);
  }
}