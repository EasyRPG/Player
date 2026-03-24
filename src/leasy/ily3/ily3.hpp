#pragma once

#include <filesystem>
#include "../ul2/state.hpp"

namespace leasy::ily3 {
  namespace global {
    extern ul2::lstate state;
  }

  void boot(const std::filesystem::path &p);
  void setup(void);
  void setup_lua(void);
}