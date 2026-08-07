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

#include <cctype>
#include <string>
#include <sstream>
#include <algorithm>

namespace leasy::kits {  
  inline std::string replace(std::string str, const std::string& from, const std::string& to) {
    if (from.empty()) return str;
    
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
      str.replace(start_pos, from.length(), to);
      start_pos += to.length();
    }
    
    return str;
  }


  template <typename T>
  T from_string_or_default(const std::string& str) {
    std::istringstream iss(str);
    T value{};

    if ((iss >> value) && (iss >> std::ws).eof()) return value;

    return T{};
  }


  inline std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
  }

  inline std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
  }

  inline std::string format_bytes(const std::size_t &bytes) {
    static constexpr const char* units[] = {
      "B","KiB","MiB","GiB","TiB","PiB"
    };

    double value = bytes;
    int unit = 0;

    while (value >= 1024.0 && unit < 5) {
      value /= 1024.0;
      ++unit;
    }

    std::ostringstream ss;

    if (unit == 0)
      ss << bytes;
    else if (value >= 100)
      ss << std::fixed << std::setprecision(0) << value;
    else if (value >= 10)
      ss << std::fixed << std::setprecision(1) << value;
    else
      ss << std::fixed << std::setprecision(2) << value;

    ss << ' ' << units[unit];
    return ss.str();
  }
}