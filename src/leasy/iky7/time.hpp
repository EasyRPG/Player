/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/


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
}

