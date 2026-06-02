#include <filesystem>
#include <iostream>
#include <string>

#include "ldebug.hpp"
#include "lio.hpp"
#include "ily3/ily3.hpp"

#include "iky7/time.hpp"

namespace leasy {
  namespace app {
    void ready(void);
    
    void lmain() {
      LDBG("sessions started!");

      iky7::time t;
      t.then([](auto t) {
         std::cout << "hi" << std::endl;
       })
       .wait(1000)
       .then([](auto t) {
         std::cout << "helo Melda <3 (ILSYMMMM)" << std::end;
       });

      throw;
			printinf("current_directory: " << std::filesystem::current_path());
      ily3::setup();
      ily3::setup_lua();
      ily3::boot(std::filesystem::current_path());
			// TODO: add the fs::current_path() to glob
      ready();
    }
  }
}
