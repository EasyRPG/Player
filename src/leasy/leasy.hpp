#pragma once

#include "ily3/ily3.hpp"

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
    void process(void);

    /** @brief This function is called before exiting (maybe, let's hope guys). */
    void exit(void); 

    /** @brief This function is called when drawing. (i swear) */
    void draw(Bitmap */* map */);

    void lmain(void);

    template <class... Args>
    inline void call(const std::string &name, Args...args) {
      ily3::global::state.call<void>(name, args...);
    }

    template <typename R, class... Args>
    inline R call(const std::string &name, Args...args) {
      return ily3::global::state.call(name, args...);
    }
  }
}