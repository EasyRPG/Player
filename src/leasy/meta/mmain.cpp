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

#include <memory>

#include "main.hpp"
#include "scene.h"
#include "scene_logo.h"
#include "metascene.hpp"
#include "../lio.hpp"
#include "../iky7/nameof.hpp"
#include "node/node2d.hpp"

#include "../cli/cli.hpp"

namespace leasy::meta2 {
  bool is_meta2_enabled = false;
  
  struct info {};

  void setmeta2(bool v) {
    is_meta2_enabled = v;
    if (v) {
      io().System.writeln(nameof<info>(), "::", __func__, ": using meta2");
    }
  }
}

namespace {
  using namespace leasy;

  auto ok = []() {
    cli::addcli("--meta2", [](auto) {
      meta2::setmeta2(true);
    });

    return false;
  }();
}
