#pragma once

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
  }
}