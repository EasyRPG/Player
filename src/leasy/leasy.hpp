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

#include <memory>
#include <unordered_set>

#include "ily3/ily3.hpp"
#include "baseui.h"
#include "dec.h"

namespace leasy {

  namespace app {
    /** 
     * @brief This returns true only if an internal error happens, or a command invokes
     * an exit-ending situation. Otherwise, this won't return if you SIG. 
     */
    bool exit_requested();

    /** 
     * @brief This asks the leasy engine for exiting. This won't make the process exit directly.
     */
    void request_exit();

    /** @brief This function is called after that EasyRPG is initialized. */
    void ready(void);

    /** @brief This function is called everytime EasyRPG updates. */
    void process();

    /** @brief This function is called before exiting (maybe, let's hope guys). */
    void exit(void); 

    /** @brief This function is called when drawing. (i swear) */
    void draw(Bitmap */* map */);

    void lmain(const std::vector<std::string>&);

    void disable();
    bool is_enabled();

    template <class... Args>
    inline void call(const std::string &name, Args...args) {
      if (! is_enabled()) return;
      ily3::global::state.call<void>(name, args...);
    }
    
    template <typename R, class... Args>
    inline R call(const std::string &name, Args...args) {
      if (! is_enabled()) return;
      return ily3::global::state.call(name, args...);
    }
  }
}

