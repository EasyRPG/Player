#include "clock.h"
#include "leasy/metadata/Domain.hpp"

namespace {
  using namespace leasy::metadata;
  auto preload = ([]() {
    auto klass = make_class<Platform_Clock>()
      .method("Name", Platform_Clock::Name)
      .method("now", Platform_Clock::now)
      .done();

    AppDomain().getAssemblyOrCreate<BuiltInAssembly>("EasyRPGPlayer")->addType<Platform_Clock>(klass);
    return false;
  }());
}