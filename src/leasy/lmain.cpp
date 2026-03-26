#include <filesystem>
#include <iostream>
#include <string>

#include "ldebug.hpp"
#include "lio.hpp"
#include "ily3/ily3.hpp"

namespace leasy {
  namespace app {
    void ready(void);
    
    void lmain() {
      LDBG("(lmain...)");
			printinf("current_directory: " << std::filesystem::current_path());
      ily3::setup();
      ily3::setup_lua();
      ily3::boot(std::filesystem::current_path());
			// TODO: add the fs::current_path() to glob
      ready();
    }
  }
}
