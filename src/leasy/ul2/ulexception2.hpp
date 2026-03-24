#pragma once

#include <string>
#include <stdexcept>

namespace leasy::ul2 {
  /** 
   * @brief Represents an internal error in ul2 systems.
   */
  class ulexception2 : public std::exception {
  private:
    std::string _what;
    std::string _from;
    std::string _whut;

  public:
    /** @brief Builds an exception.
     * @param F the thrower
     * @param W the message
     */
    ulexception2(const std::string&F, const std::string&W);
    const char *what() const noexcept override;
    virtual std::string whut() const noexcept;
  };
}

#define ulthrow(X) throw leasy::ul2::ulexception2(__FILE__ + std::string(":") + (std::to_string(__LINE__) + ":") + __func__, X)