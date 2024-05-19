#include "json_helper.h"
#include "output.h"
#include <nlohmann/json.hpp>

namespace Json_Helper {
	std::string invalid_str = "<<INVALID_OUTPUT>>";
	std::string GetValue(const std::string& jsonData, const std::string& jsonPath) {
		try {
			nlohmann::json jsonObj = nlohmann::json::parse(jsonData);
			nlohmann::json* currentObj = &jsonObj;

			std::string mutableJsonPath = jsonPath;
			size_t pos = 0;
			std::string token;
			while ((pos = mutableJsonPath.find('.')) != std::string::npos) {
				token = mutableJsonPath.substr(0, pos);
				if (token.back() == ']') {
					size_t bracketPos = token.find('[');
					std::string arrayName = token.substr(0, bracketPos);
					int index = std::stoi(token.substr(bracketPos + 1, token.length() - bracketPos - 2));
					if (currentObj->find(arrayName) == currentObj->end() || !(*currentObj)[arrayName].is_array() || index >= (*currentObj)[arrayName].size()) {
						Output::Warning("JSON_ERROR - Invalid path: {}", jsonPath);
						return invalid_str;
					}
					currentObj = &((*currentObj)[arrayName][index]);
				}
				else {
					if (currentObj->find(token) == currentObj->end()) {
						Output::Warning("JSON_ERROR - Invalid path: {}", jsonPath);
						return invalid_str;
					}
					currentObj = &((*currentObj)[token]);
				}
				mutableJsonPath.erase(0, pos + 1);
			}

			if (!mutableJsonPath.empty()) {
				if (mutableJsonPath.back() == ']') {
					size_t bracketPos = mutableJsonPath.find('[');
					std::string arrayName = mutableJsonPath.substr(0, bracketPos);
					int index = std::stoi(mutableJsonPath.substr(bracketPos + 1, mutableJsonPath.length() - bracketPos - 2));
					if (currentObj->find(arrayName) == currentObj->end() || !(*currentObj)[arrayName].is_array() || index >= (*currentObj)[arrayName].size()) {
						Output::Warning("JSON_ERROR - Invalid path: {}", jsonPath);
						return invalid_str;
					}
					currentObj = &((*currentObj)[arrayName][index]);
				}
				else {
					if (currentObj->find(mutableJsonPath) == currentObj->end()) {
						Output::Warning("JSON_ERROR - Invalid path: {}", jsonPath);
						return invalid_str;
					}
					currentObj = &((*currentObj)[mutableJsonPath]);
				}
			}

			if (currentObj->is_string()) {
				return currentObj->get<std::string>();
			}
			else if (currentObj->is_number_integer()) {
				return std::to_string(currentObj->get<int>());
			}
			else if (currentObj->is_number_float()) {
				return std::to_string(currentObj->get<float>());
			}
			else if (currentObj->is_boolean()) {
				return currentObj->get<bool>() ? "true" : "false";
			}
			else {
				return currentObj->dump();
			}
		}
		catch (const std::exception& e) {
			Output::Warning("JSON_ERROR - {}", e.what());
			return invalid_str;
		}
	}

	std::string SetValue(const std::string& jsonData, const std::string& jsonPath, const std::string& value) {
		try {
			nlohmann::json jsonObj = nlohmann::json::parse(jsonData);
			nlohmann::json* currentObj = &jsonObj;

			std::string mutableJsonPath = jsonPath;
			size_t pos = 0;
			std::string token;
			while ((pos = mutableJsonPath.find('.')) != std::string::npos) {
				token = mutableJsonPath.substr(0, pos);
				if (token.back() == ']') {
					size_t bracketPos = token.find('[');
					std::string arrayName = token.substr(0, bracketPos);
					int index = std::stoi(token.substr(bracketPos + 1, token.length() - bracketPos - 2));
					currentObj = &((*currentObj)[arrayName][index]);
				}
				else {
					currentObj = &((*currentObj)[token]);
				}
				mutableJsonPath.erase(0, pos + 1);
			}

			if (mutableJsonPath.back() == ']') {
				size_t bracketPos = mutableJsonPath.find('[');
				std::string arrayName = mutableJsonPath.substr(0, bracketPos);
				int index = std::stoi(mutableJsonPath.substr(bracketPos + 1, mutableJsonPath.length() - bracketPos - 2));
				(*currentObj)[arrayName][index] = value;
			}
			else {
				(*currentObj)[mutableJsonPath] = value;
			}

			return jsonObj.dump();
		}
		catch (const std::exception& e) {
			Output::Warning("JSON_ERROR - {}", e.what());
			return jsonData;
		}
	}
}
