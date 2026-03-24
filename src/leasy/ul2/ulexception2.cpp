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