#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <functional>

namespace leasy::iky7 {
  class time {
  private:
    std::chrono::duration<uint64_t> p;

  public:
    inline time() : p(std::chrono::high_resolution_clock::now().time_since_epoch().count()) {}
    inline time then(const std::function<void(time)> &f) const { // f stands for "FUCK YOU MEAN" (lol)
      f(*this);
      return time(); // this gon' return when the function actually finishes executing (lol yo dum)
    }

    inline time wait(uint64_t ms) const {
      std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      return time();
    }

    inline std::chrono::duration<uint64_t> internalpoint() const { return this->p; }
  };
  
  // TODO: timeSinceStart!
}
