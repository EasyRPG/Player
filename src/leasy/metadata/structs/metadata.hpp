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

//#include "../ul2/state.hpp"

#include "object.hpp"

namespace leasy::ul2 {
  class lstate;
}

namespace leasy::metadata {
  class Data {
  public:
    virtual ~Data() = default;

    [[nodiscard]] virtual Object dump() const { return {}; }
    virtual void bind(ul2::lstate&) const {}
  };

  class SizeDescriptor {
  public:
    virtual ~SizeDescriptor() = default;

    [[nodiscard]] virtual size_t getMetadataSize() const = 0;
  };

  inline size_t getStringRealSize(const std::string &k) {
    return sizeof(k) + (sizeof(kits::flat_t<decltype(k)>::value_type) * k.capacity());
  }
}

