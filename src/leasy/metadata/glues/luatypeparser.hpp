#pragma once

#include <string>
#include "leasy/types/string.hpp"

namespace leasy::metadata::glues::utils {
  inline auto qualify(const std::string &qualifier, const std::string &T) {
    return qualifier + "<" + T + ">";
  }

  inline auto isFinalized(const std::string& s) {
    return s.find("const") == std::string::npos &&
           s.find("volatile") == std::string::npos &&
           s.find('*') == std::string::npos &&
           s.find('&') == std::string::npos;
  }

  inline String getLuaQualifiedForm(const String &type) {
    if (type == "&") return "ref";
    if (type == "&&") return "rvalue_ref";
    if (type == "*") return "ptr";
    return type;
  }

  inline auto transformType(const String &type) {
    if (isFinalized(type)) return type;
    auto myString = type;
    std::stack<String> st;
    String finalType = "int"; // int is assumed as default type ... idk why.
    bool hasFinalType = false;

    while (! isFinalized(myString)) {
      auto frontQualifier = myString.findAny({"const", "volatile"});
      if (frontQualifier.found) {
        // Next may be a type info!
        auto qual = myString.substr(frontQualifier.index, frontQualifier.string.size());
        st.emplace(qual);
        myString = myString.substr(frontQualifier.index + frontQualifier.string.size());
      }

      auto backQualifier = myString.findAny({"*", "&&", "&"});
      if (backQualifier.found) {
        if (!hasFinalType) {
          finalType = myString.substr(0, backQualifier.index);
          hasFinalType = true;
        }

        st.emplace(getLuaQualifiedForm(
            myString.substr(
                backQualifier.index,
                backQualifier.string.size()
            )
        ));

        myString = myString.substr(
            backQualifier.index + backQualifier.string.size()
        );
      } else {
        if (!hasFinalType) {
          finalType = myString;
          hasFinalType = true;
        }
      }
    }

    while (!st.empty()) {
      finalType = qualify(st.top(), finalType);
      st.pop();
    }

    return finalType;
  }
}
