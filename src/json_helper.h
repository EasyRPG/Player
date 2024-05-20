#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <string>
#include <nlohmann/json.hpp>

namespace Json_Helper {
	std::string GetValue(std::string_view json_data, std::string_view json_path);
	std::string SetValue(std::string_view json_data, std::string_view json_path, std::string_view value);
}

#endif // JSON_HELPER_H
