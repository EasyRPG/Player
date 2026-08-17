#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>

#include "../type.hpp"
#include "../function.hpp"
#include "../Assembly.hpp"
#include "leasy/metadata/Domain.hpp"

namespace leasy::metadata {

namespace {

  //
  // std::type_index
  //
  std::shared_ptr<Class> build_type_index() {
    auto c = make_class<std::type_index>();

    c.method("name", [](const std::type_index& t) -> std::string {
      return t.name();
    });

    c.method("hashCode", [](const std::type_index& t) -> std::size_t {
      return t.hash_code();
    });

    c.method("eq",
             [](const std::type_index& a,
                const std::type_index& b) {
               return a == b;
             });

    c.method("ne",
             [](const std::type_index& a,
                const std::type_index& b) {
               return a != b;
             });

    return c.done();
  }


  //
  // Class
  //
  std::shared_ptr<Class> build_class() {
    auto c = make_class<Class>();

    c.method("name", [](const Class& c) {
      return c.fullname();
    });

    c.method("cindex", [](const Class& c) {
      return c.cindex();
    });

    c.method("methods", [](const Class& c) {
      return c.methods();
    });

    c.method("dump", [](const Class& c) {
      return c.dump();
    });

    c.method("metadataSize", [](const Class& c) {
      return c.getMetadataSize();
    });

    return c.done();
  }


  //
  // Assembly
  //
  std::shared_ptr<Class> build_assembly() {
    auto c = make_class<Assembly>();

    c.method("name", [](const Assembly& a) {
      return a.name();
    });

    c.method("types", [](const Assembly& a) {
      return a.getTypes();
    });

    c.method("functions", [](const Assembly& a) {
      return a.getFunctions();
    });

    c.method("metadataSize", [](const Assembly& a) {
      return a.getMetadataSize();
    });

    c.method("getType",
             [](const Assembly& a, const std::type_index& index) {
               return a.getType(index);
             });

    c.method("getType",
             [](const Assembly& a, const std::string& name) {
               return a.getType(name);
             });

    c.method("getFunction",
             [](const Assembly& a, const std::string& name) {
               return a.getFunction(name);
             });

    c.method("dump", [](const Assembly& a) {
      return a.dump();
    });

    return c.done();
  }


  //
  // BuiltInAssembly
  //
  std::shared_ptr<Class> build_builtin_assembly() {
    auto c = make_class<BuiltInAssembly>();

    c.method("name", [](const BuiltInAssembly& a) {
      return a.name();
    });

    c.method("types", [](const BuiltInAssembly& a) {
      return a.getTypes();
    });

    c.method("functions", [](const BuiltInAssembly& a) {
      return a.getFunctions();
    });

    c.method("get_type",
             [](const BuiltInAssembly& a,
                const std::type_index& index) {
               return a.getType(index);
             });

    c.method("getType",
             [](const BuiltInAssembly& a,
                const std::string& name) {
               return a.getType(name);
             });

    c.method("getFunction",
             [](const BuiltInAssembly& a,
                const std::string& name) {
               return a.getFunction(name);
             });

    c.method("dump", [](const BuiltInAssembly& a) {
      return a.dump();
    });

    return c.done();
  }


  //
  // function_base_t
  //
  std::shared_ptr<Class> build_function() {
    auto c = make_class<function_base_t>();

    c.method("name", [](const function_base_t& f) {
      return f.name;
    });

    c.method("metadataSize", [](const function_base_t& f) {
      return f.getMetadataSize();
    });

    c.method("dump", [](const function_base_t& f) {
      return f.dump();
    });

    return c.done();
  }

}


namespace {

const auto REFLECTION_ASSEMBLY = [] {
  auto assembly = AppDomain().getAssemblyOrCreate<BuiltInAssembly>("leasy::reflection");

  assembly->addType<std::type_index>(
    build_type_index()
  );

  assembly->addType<Class>(
    build_class()
  );

  assembly->addType<Assembly>(
    build_assembly()
  );

  assembly->addType<function_base_t>(
    build_function()
  );

  return false;
}();

}

}