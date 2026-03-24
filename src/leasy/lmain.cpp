#include <iostream>
#include <string>

#include "ldebug.hpp"
#include "ily3/ily3.hpp"

namespace leasy {
  namespace app {
    void ready(void);
    
    void lmain() {
      LDBG("(lmain...)");
      ily3::setup();
      ily3::setup_lua();
      ily3::boot(std::filesystem::current_path());
      ready();
    }
  }
}
