#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <string>

namespace Json_Helper {
	std::string GetValue(const std::string& jsonData, const std::string& jsonPath);
	std::string SetValue(const std::string& jsonData, const std::string& jsonPath, const std::string& value);
}

#endif // JSON_HELPER_H
