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
 *  0xEF9087A
 * 
 * **********************************************************************/
#include <string>
#include <stdexcept>

#include "ulexception2.hpp"

namespace leasy::ul2 {

  ulexception2::ulexception2(const std::string&F, const std::string&W) {
    this->_from = std::string(F);
    this->_what = std::string(W); // I don't fucking know why the compiler said me i cant use js "=" lol
    this->_whut = F + ":\t" + W;
  }

  const char *ulexception2::what() const noexcept {
    return _whut.c_str();
  }

  std::string ulexception2::whut() const noexcept {
    return _whut;
  }
}