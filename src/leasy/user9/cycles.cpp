#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#include "bitmap.h"

namespace leasy::user9 {
  namespace share {
    std::unordered_map<unsigned long long, std::weak_ptr<std::function<void(double)>>> holdupdates;
    std::unordered_map<unsigned long long, std::weak_ptr<std::function<void(Bitmap&)>>> holddraws;
    std::vector<std::function<void(void)>> atexitholds;
    std::vector<std::function<void(double)>> holdnextupdate;
    std::vector<std::function<void(Bitmap&)>> holdnextdraw;
  }

  using namespace share;

  static unsigned long long idc = 0;

  std::shared_ptr<std::function<void(double)>> onupdate(const std::function<void(double)> &f) {
    std::shared_ptr<std::function<void(double)>> ptr = std::make_shared<std::function<void(double)>>(f);

    holdupdates[idc++] = ptr;

    return std::move(ptr);
  }

  std::shared_ptr<std::function<void(Bitmap&)>> ondraw(const std::function<void(Bitmap&)> &f) {
    std::shared_ptr<std::function<void(Bitmap&)>> ptr = std::make_shared<std::function<void(Bitmap&)>>(f);

    holddraws[idc++] = ptr;

    return std::move(ptr);
  }


  void nextupdate(const std::function<void(double)> &f) {
    holdnextupdate.emplace_back(f);
  }
  
  void nextdraw(const std::function<void(Bitmap&)> &f) {
    holdnextdraw.emplace_back(f);
  }
  
  
  void onexit(const std::function<void(void)> &f) {
    atexitholds.emplace_back(f);
  }
}