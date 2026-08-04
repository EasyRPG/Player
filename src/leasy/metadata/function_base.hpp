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

#include <any>
#include <vector>
#include <functional>

#include "../iky7/nameof.hpp"
#include "../lio.hpp"
#include "../lua/lua.hpp"

#include "metadata.hpp"

namespace leasy::metadata {
  class Class; // What the fuck.

  /** Small notes for @wys (myself)
   * So basically, after the issue #3, i saw that yes indeed, types are ... weird.
   * We have the native type, and also classes ... But arguments uses types ... So classes are useless?
   * So my initiative was to concat everything in Class.
   */

   /**
    * The main goal here is to abstract everything that we can call.
    * Indeed, in previous version, and still in the current one, we have a distinct difference between
    * a simple function, and an overloaded one.
    * However, maybe over the time, callables will evolve. I'll simply abstract everything in a base class,
    * and then, extend it etc. This will sadly drop my Callable wrapper (that before, used to flatten the syntax and internally
    * use a shared_ptr<> for a nicer syntax. But i still can hide this.)
    * 
    * All my apoligies for not writing in a proper english.
    */
   class function_base_t;

  class function_base_t : public Data {
  public:
    std::string name;

    inline virtual std::pair<bool, std::string> is_callable(std::vector<std::any>&) const {
      return {false, "You're trying to call a default-virtual function !"}; 
    }

    inline virtual std::any call(std::vector<std::any>&) const {
      return {}; // I prefer an API that fails over virtual methods rather than making weird pure virtual things.
    }

    inline virtual std::function<int(lua_State*)> lua() const {
      return [](lua_State*) -> int {
        // And making warnings is the best way to debug!
        io().Warning.writeln(__func__, ": unimplemented binding was called on base ", nameof<decltype(*this)>());
        return 0;
      };
    }

    virtual ~function_base_t() = default;
  };
}