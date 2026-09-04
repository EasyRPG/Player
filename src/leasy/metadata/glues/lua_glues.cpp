/**
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
 *  You may see the evolution of this file at
 *  https://github.com/wys-prog/leasy.
 *
 *  0xEF9087A@wys-prog
 *  https://github.com/wys-prog/leasy
 *
 *  **************************************************************************
 */

//
// Created by @wys on 08/08/2026.
//

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

// Converts a C++ type name into something that can safely be used
// as an EmmyLua type name.
//
// Examples:
//   const Foo     -> const_Foo
//   Foo*          -> Fooptr
//   Foo&          -> Fooref
//   Foo&&         -> Foorref
//   volatile Foo  -> volatile_Foo
//
// Spaces are removed because the resulting value is used directly
// inside EmmyLua annotations.
static auto cleanLuaType(String value) {
  return value
    .replace("&&", "rref")
    .replace("&", "ref")
    .replace("*", "ptr")
    .replace("const", "const_")
    .replace("volatile", "volatile_")
    .replace(" ", "");
}

static String ensurePath(const String &path) {
  const auto parts = path.split('.');
  String currentPath = "_G";
  String code;

  for (const auto &part : parts) {
    if (canBeLuaFunctionName(part))
      currentPath += "." + part;
    else
      currentPath += "['" + part + "']";

    code += currentPath + " = " + currentPath + " or {}\n";
  }

  return code + '\n';
}

static String ensurePrefix(const String &prefix) {
  if (!prefix.empty() && prefix.back() == '.')
    return prefix;

  return prefix + '.';
}

static String luaClassName(const String &prefix, const Class &cl) {
  if (prefix.empty())
    return String(cl.fullname()).replace("::", ".");

  return String(ensurePrefix(prefix) + cl.fullname())
    .replace("::", ".");
}

static String luaClassMangledName(const String &prefix, const Class &cl) {
  return String(ensurePrefix(prefix) + cl.cindex().name())
    .replace("::", ".");
}

static String constructLuaName(const String &name) {
  const auto parts = name.split('.');
  String result;

  for (const auto &part : parts) {
    if (canBeLuaFunctionName(part))
      result += "." + part;
    else
      result += "['" + part + "']";
  }

  if (result.empty())
    return name;

  return result.substr(1);
}

static void includeFunctionOverloads(
  const String &prefix,
  const function_base_t &func,
  std::ostream &ostream) {

  for (const auto &signature : func.getSignatures()) {
    std::vector<String> args;

    for (std::size_t i = 0; i < signature.arguments.size(); ++i) {
      const auto name = cleanLuaType(
        luaClassName("", *signature.arguments[i])
      );

      args.emplace_back(
        fmt::format("arg{}: {}{}", i, prefix, name)
      );
    }

    const auto ret = cleanLuaType(
      luaClassName("", *signature.returnType)
    );

    ostream << fmt::format(
      "---@overload fun({}):{}{}",
      String::join(args, ", "),
      prefix,
      ret
    ) << '\n';
  }
}

static void includeType(
  const String &prefix,
  const Assembly &assembly,
  const Class &type,
  std::ostream &ostream) {

  const String classBaseName = luaClassName(prefix, type);

  const auto lastDot = classBaseName.rfind('.');

  String classQualification = classBaseName;

  if (lastDot != String::npos)
    classQualification = classBaseName.substr(0, lastDot);

  const String myLuaClassName = cleanLuaType(classBaseName);

  const String mangledBaseName =
    luaClassMangledName(prefix + "." + assembly.name(), type);

  ostream << ensurePath(classQualification);

  ostream << "\n---@class "
           << myLuaClassName
           << '\n';

  ostream << constructLuaName(classBaseName)
           << " = {}\n\n";

  for (const auto &[name, method] : type.methods()) {
    const auto functionName =
      constructLuaName(classBaseName + "." + name);

    const auto internalFunctionName =
      constructLuaName(mangledBaseName + "." + name);

    includeFunctionOverloads(prefix, *method, ostream);

    ostream << fmt::format(
      "{} = function(...)\n"
      "  return {}(...)\n"
      "end\n",
      functionName,
      internalFunctionName
    ) << '\n';
  }
}

static void includeAssembly(
  const String &initialPrefix,
  const Assembly &assembly,
  std::ostream &ostream) {

  const String prefix = ensurePrefix(initialPrefix);

  ostream << "--- from Assembly '"
           << assembly.name()
           << "'\n";

  ostream << ensurePath(
    kits::replace(prefix + assembly.name(), "::", ".")
  );

  for (const auto &type : assembly.getTypes())
    includeType(prefix, assembly, *type, ostream);

  for (const auto &fun : assembly.getFunctions()) {
    includeFunctionOverloads(prefix, *fun, ostream);

    const auto name = constructLuaName(
      kits::replace(prefix + fun->name, "::", ".")
    );

    ostream << fmt::format(
      "function {}(...) end",
      name
    ) << '\n';
  }
}

void generateLuaGlue(
  const std::string &prefix,
  const Domain &domain,
  const std::filesystem::path &p) {

  std::vector<String> requires;

  for (const auto &assembly : domain.getAssemblies()) {
    const auto assemblyPath = String(assembly->name())
      .replace("::", "/")
      .replace(".", "/");

    auto parts = kits::split(
      (p / assemblyPath.c_str()).string(),
      '/'
    );

    if (parts.empty())
      parts = {p.string(), "unknown"};

    const auto filename = parts.back() + ".lua";

    const auto filepath =
      kits::join({parts.begin(), parts.end() - 1}, "/");

    if (!std::filesystem::exists(filepath))
      std::filesystem::create_directories(filepath);

    const auto fileFullpath =
      std::filesystem::path(filepath) / filename;

    std::ofstream ostream(fileFullpath);

    if (!ostream) {
      throw std::runtime_error(
        "unable to open dump file: " + p.string()
      );
    }

    ostream << ensurePath(prefix);

    includeAssembly(prefix, *assembly, ostream);

    io().System.writeln(
      "Xglue: exported assembly glues for assembly ",
      filepath
    );

    requires.push_back(
      String(fileFullpath.string())
        .replace("/", ".")
        .replace("\\", ".")
    );
  }

  std::ofstream ostream(p / "appdomain.lua");

  for (auto req : requires) {
    ostream << "local _ = require('"
             << req.replace(".lua", "")
             << "')\n";
  }
}

} // namespace leasy::metadata::glues
