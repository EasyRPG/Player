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

  inline std::string join(const std::vector<std::string>& elements, const std::string& sep, const bool includeLast = false) {
    std::string result;

    for (size_t i = 0; i < elements.size(); i++) {
      result += elements[i];

      if (i + 1 < elements.size() || includeLast)
        result += sep;
    }

    return result;
  }

  inline std::vector<std::string> split(const std::string &name, char sep = '.') {
    std::vector<std::string> out;
    std::stringstream ss(name);
    std::string item;

    while (std::getline(ss, item, sep)) {
      if (!item.empty()) out.push_back(item);
    }

    return out;
  }

  struct SubStringInfo {
    std::string string;
    size_t      index;
    bool        found;
  };

  inline SubStringInfo find_any(const std::string &string, const std::vector<std::string> &elements) {
    for (const auto&e: elements) {
      if (string.find(e) != std::string::npos) {
        return {
          .string = e,
          .index = string.find(e),
          .found = true,
        };
      }
    }

    return {
      .string = "",
      .index = std::string::npos,
      .found = false,
    };
  }
}
