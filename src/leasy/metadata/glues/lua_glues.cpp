//
// Created by @wys on 08/08/2026.
//

#include <cctype>
#include <fmt/format.h>

#include "lua_glues.hpp"
#include "leasy/kits/strings.hpp"
#include "luatypeparser.hpp"

namespace leasy::metadata::glues {
  static bool canBeLuaFunctionName(const std::string &name) {
    if (name.empty())
      return false;

    const auto first = static_cast<unsigned char>(name.front());

    if (!(std::isalpha(first) || first == '_'))
      return false;

    for (std::size_t i = 1; i < name.size(); ++i) {
      const auto c = static_cast<unsigned char>(name[i]);

      if (!(std::isalnum(c) || c == '_'))
        return false;
    }

    return true;
  }

  static auto removeSpaces(const std::string &value) {
    return kits::replace(value, " ", "");
  }

  static String ensurePath(const String &path) {
    const auto parts = path.split('.');

    String currentPath = "_G";
    String code;

    for (const auto &part: parts) {
      if (! canBeLuaFunctionName(part)) {
        currentPath += "['" + part + "']";
      } else {
        currentPath += "." + part;
      }
      code += currentPath + " = " + currentPath + " or {}\n";
    }

    return code + "\n";
  }

  static String ensurePrefix(const String &prefix) {
    if (!prefix.empty() && prefix.back() == '.')
      return prefix;

    return prefix + '.';
  }

  static String luaClassName(const String &prefix, const Class &cl) {
    const String myPrefix = ensurePrefix(prefix);
    return String(myPrefix + cl.fullname()).replace("::", ".");
  }

  static String luaClassMangledName(const String &prefix, const Class &cl) {
    const String myPrefix = ensurePrefix(prefix);
    return String(myPrefix + cl.cindex().name()).replace("::", ".");
  }

  static String constructLuaName(const String &name) {
    auto parts = name.split('.');
    String end;

    for (const auto &part: parts) {
      if (canBeLuaFunctionName(part)) {
        end += "." + part;
      } else {
        end += "['" + part + "']";
      }
    }

    if (end.empty()) return name;

    return end.substr(1);
  }

  static void includeFunctionOverloads(const String &prefix, const function_base_t &func, std::ostream &ostream) {
    auto signatures = func.getSignatures();

    for (const auto &signature: signatures) {
      std::vector<String> args;
      for (size_t i = 0; i < signature.arguments.size(); ++i) {
        auto name = removeSpaces(utils::transformType(luaClassName(prefix, *signature.arguments[i])));
        args.emplace_back(fmt::format("arg{}: {}", i, name));
      }
      auto ret = removeSpaces(utils::transformType(luaClassName(prefix, *signature.returnType)));
      ostream << fmt::format("---@overload fun({}):{}", String::join(args, ", "), ret) << "\n";
    }
  }

  static void includeType(const String &prefix, const Class &type, std::ostream &ostream) {
    String classBaseName = luaClassName(prefix, type);
    auto lastDot = classBaseName.rfind('.');
    String classQualification = classBaseName;
    String luaClassName = removeSpaces(utils::transformType(classBaseName));

    if (lastDot != String::npos) {
      classQualification = classBaseName.substr(0, lastDot);
    }

    String mangledBaseName = luaClassMangledName(prefix, type);
    ostream << ensurePath(classQualification) << "\n";
    ostream << "---@class " << luaClassName << "\n";
    ostream << constructLuaName(classBaseName) << " = {}\n\n";

    for (const auto &[name, method]: type.methods()) {
      auto functionName = constructLuaName(classBaseName + "." + name);
      auto internalFunctionName = constructLuaName(mangledBaseName + "." + name);

      includeFunctionOverloads(prefix, *method, ostream);
      ostream
      << fmt::format("{} = function(...)\n  return {}(...)\nend", functionName, internalFunctionName)
      << std::endl;
    }
  }

  static void includeAssembly(const String &initialPrefix, const Assembly &assembly, std::ostream &ostream) {
    String prefix = ensurePrefix(initialPrefix);
    ostream << "--- from Assembly '" << assembly.name() << "'"  << std::endl;

    for (const auto&type: assembly.getTypes()) {
      includeType(prefix, *type, ostream);
    }
  }

  void generateLuaGlue(
    const std::string &prefix,
    const Domain &domain,
    std::ostream &ostream) {
    ostream << ensurePath(prefix);

    io().Warning.writeln(utils::transformType(std::string(nameof<int******>())));

    for (const auto &assembly: domain.getAssemblies()) {
      includeAssembly(prefix, *assembly, ostream);
    }
  }
} // namespace leasy::metadata::glues
