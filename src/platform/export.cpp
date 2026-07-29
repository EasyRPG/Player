#include "clock.h"
#include "leasy/metadata/namespace.hpp"

namespace {
  using namespace leasy::metadata;
  auto preload = ([]() {
    auto klass = make_class<Platform_Clock>()
      .method("Name", Platform_Clock::Name)
      .method("now", Platform_Clock::now)
      .done();

    EasyRPG().add(klass);
    return false;
  }());
}