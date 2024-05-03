#include "json_helper.h"
#include "external/picojson.h"
#include "output.h"
#include <vector>

namespace Json_Helper {
	const std::string kInvalidOutput = "<<INVALID_OUTPUT>>";

	std::vector<std::string> SplitPath(const std::string& jsonPath) {
		std::istringstream iss(jsonPath);
		std::vector<std::string> pathParts;
		std::string part;
		while (std::getline(iss, part, '.')) {
			pathParts.push_back(part);
		}
		return pathParts;
	}

	picojson::value* GetValuePointer(picojson::value& jsonValue, const std::vector<std::string>& pathParts, bool allowCreation) {
		picojson::value* currentValue = &jsonValue;

		for (const auto& part : pathParts) {
			std::string arrayName;
			int arrayIndex = -1;
			bool inArray = false;

			for (char c : part) {
				if (!inArray) {
					if (c == '[') {
						inArray = true;
						arrayIndex = 0;
					}
					else {
						arrayName += c;
					}
				}
				else {
					if (c == ']') {
						break;
					}
					else {
						arrayIndex = arrayIndex * 10 + (c - '0');
					}
				}
			}

			if (inArray) {
				if (!currentValue->is<picojson::object>()) {
					if (allowCreation) {
						*currentValue = picojson::value(picojson::object());
					}
					else {
						Output::Warning("JSON_ERROR - Invalid JSON type for array: {}", arrayName);
						return nullptr;
					}
				}

				auto& obj = currentValue->get<picojson::object>();
				auto arrayIt = obj.find(arrayName);

				if (arrayIt == obj.end()) {
					if (allowCreation) {
						obj.emplace(arrayName, picojson::value(picojson::array()));
						arrayIt = obj.find(arrayName);
					}
					else {
						Output::Warning("JSON_ERROR - Array not found: {}", arrayName);
						return nullptr;
					}
				}

				auto& arr = arrayIt->second.get<picojson::array>();

				if (arrayIndex >= static_cast<int>(arr.size())) {
					if (allowCreation) {
						arr.resize(arrayIndex + 1);
					}
					else {
						Output::Warning("JSON_ERROR - Array index out of bounds: {}", part);
						return nullptr;
					}
				}

				currentValue = &arr[arrayIndex];
			}
			else {
				if (!currentValue->is<picojson::object>()) {
					if (allowCreation) {
						*currentValue = picojson::value(picojson::object());
					}
					else {
						Output::Warning("JSON_ERROR - Invalid JSON type for path: {}", part);
						return nullptr;
					}
				}

				auto& obj = currentValue->get<picojson::object>();
				auto objIt = obj.find(arrayName);

				if (objIt == obj.end()) {
					if (allowCreation) {
						obj.emplace(arrayName, picojson::value(picojson::object()));
						objIt = obj.find(arrayName);
					}
					else {
						Output::Warning("JSON_ERROR - Object key not found: {}", part);
						return nullptr;
					}
				}

				currentValue = &objIt->second;
			}
		}

		return currentValue;
	}

	std::string GetValue(const std::string& jsonData, const std::string& jsonPath) {
		picojson::value jsonValue;
		std::string err = picojson::parse(jsonValue, jsonData);

		if (!err.empty()) {
			Output::Warning("JSON_ERROR - JSON parsing error: {}", err);
			return kInvalidOutput;
		}

		auto pathParts = SplitPath(jsonPath);
		auto valuePtr = GetValuePointer(jsonValue, pathParts, false);

		if (valuePtr == nullptr || !valuePtr->is<std::string>()) {
			Output::Warning("JSON_ERROR - Value not found or not a string");
			return kInvalidOutput;
		}

		return valuePtr->get<std::string>();
	}

	std::string SetValue(const std::string& jsonData, const std::string& jsonPath, const std::string& value) {
		picojson::value jsonValue;
		std::string err = picojson::parse(jsonValue, jsonData);

		if (!err.empty()) {
			Output::Warning("JSON_ERROR - JSON parsing error: {}", err);
			return kInvalidOutput;
		}

		auto pathParts = SplitPath(jsonPath);
		auto valuePtr = GetValuePointer(jsonValue, pathParts, true);

		if (valuePtr == nullptr) {
			Output::Warning("JSON_ERROR - Unable to create value");
			return kInvalidOutput;
		}

		*valuePtr = picojson::value(value);

		return picojson::value(jsonValue).serialize();
	}
}
