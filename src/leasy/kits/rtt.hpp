#pragma once

#include <any>
#include <memory>
#include <string>
#include <vector>
#include <typeindex>

namespace leasy::metadata {
  class Class;
  bool is_same_type(const std::type_index &cindex, const std::shared_ptr<Class> &classptr);
  std::shared_ptr<Class> typeidof(const std::type_index&);
}

namespace leasy::kits {

  inline std::vector<std::shared_ptr<metadata::Class>> extract_types(const std::vector<std::any> &objects) {
    std::vector<std::shared_ptr<metadata::Class>> indexes;
    indexes.reserve(objects.size());

    for (const auto& object: objects) {
      indexes.push_back(metadata::typeidof(object.type()));
    }

    return indexes;
  }

  inline std::pair<bool, std::string> is_callable_with(const std::vector<std::shared_ptr<metadata::Class>> &types, const std::vector<std::any> &args) {
    if (types.size() != args.size()) return {false, "argument size mismatch! expected " + std::to_string(types.size()) + " got " + std::to_string(args.size())};
    else {
      for (size_t i = 0; i < types.size(); i++) {
        if (! metadata::is_same_type(args[i].type(), types[i])) return {
          false, "argument #" + std::to_string(i) + " mismatch!"
        };
      }
    }

    return {true, "success"};
  }

  template<typename Tuple, std::size_t... Is>
  std::vector<std::shared_ptr<metadata::Class>>
  tuple_types_impl(std::index_sequence<Is...>)
  {
    return {
      metadata::typeidof(typeid(std::tuple_element_t<Is, Tuple>))...
    };
  }

  template<typename Tuple>
  std::vector<std::shared_ptr<metadata::Class>> tuple_types()
  {
    return tuple_types_impl<Tuple>(
        std::make_index_sequence<std::tuple_size_v<Tuple>>{}
    );
  }
}