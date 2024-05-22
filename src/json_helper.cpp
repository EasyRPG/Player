/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#include "json_helper.h"

#ifdef HAVE_NLOHMANN_JSON

#include "output.h"
#include <sstream>
#include <unordered_map>

namespace Json_Helper {

	const std::string k_invalid_str = "<<INVALID_OUTPUT>>";

	std::unordered_map<std::string, nlohmann::json> json_cache;

	nlohmann::json* GetObjectAtPath(nlohmann::json& json_obj, std::string_view json_path, bool allow_path_creation = false) {
		nlohmann::json* current_obj = &json_obj;

		std::stringstream path_stream(json_path.data());
		std::string token;

		while (std::getline(path_stream, token, '.')) {
			if (token.back() == ']') {
				size_t bracket_pos = token.find('[');
				std::string array_name = token.substr(0, bracket_pos);
				int index = std::stoi(token.substr(bracket_pos + 1, token.length() - bracket_pos - 2));

				if (!current_obj->contains(array_name)) {
					if (allow_path_creation) {
						(*current_obj)[array_name] = nlohmann::json::array();
					}
					else {
						return nullptr;
					}
				}

				nlohmann::json& array_obj = (*current_obj)[array_name];

				if (index >= array_obj.size()) {
					if (allow_path_creation) {
						array_obj.get_ref<nlohmann::json::array_t&>().resize(index + 1);
					}
					else {
						return nullptr;
					}
				}

				current_obj = &(array_obj[index]);
			}
			else {
				if (!current_obj->contains(token)) {
					if (allow_path_creation) {
						(*current_obj)[token] = nlohmann::json::object();
					}
					else {
						return nullptr;
					}
				}

				current_obj = &((*current_obj)[token]);
			}
		}

		return current_obj;
	}

	std::string GetValueAsString(const nlohmann::json& json_obj) {
		std::string result;

		if (json_obj.is_string()) {
			result = json_obj.get<std::string>();
		}
		else if (json_obj.is_number_integer()) {
			result = std::to_string(json_obj.get<int>());
		}
		else if (json_obj.is_number_float()) {
			result = std::to_string(json_obj.get<float>());
		}
		else if (json_obj.is_boolean()) {
			result = json_obj.get<bool>() ? "true" : "false";
		}
		else {
			result = json_obj.dump();
		}

		return result;
	}

	std::string GetValue(std::string_view json_data, std::string_view json_path) {
		try {
			nlohmann::json json_obj;
			auto it = json_cache.find(json_data.data());

			if (it != json_cache.end()) {
				json_obj = it->second;
			}
			else {
				json_obj = nlohmann::json::parse(json_data);
				json_cache[json_data.data()] = json_obj;
			}

			nlohmann::json* current_obj = GetObjectAtPath(json_obj, json_path);

			if (current_obj == nullptr) {
				Output::Warning("JSON_ERROR - Invalid path: {}", json_path);
				return k_invalid_str;
			}

			return GetValueAsString(*current_obj);
		}
		catch (const std::exception& e) {
			Output::Warning("JSON_ERROR - {}", e.what());
			return k_invalid_str;
		}
	}

	std::string SetValue(std::string_view json_data, std::string_view json_path, std::string_view value) {
		try {
			nlohmann::json json_obj;
			auto it = json_cache.find(json_data.data());

			if (it != json_cache.end()) {
				json_obj = it->second;
			}
			else {
				json_obj = nlohmann::json::parse(json_data);
				json_cache[json_data.data()] = json_obj;
			}

			nlohmann::json* current_obj = GetObjectAtPath(json_obj, json_path, true);

			if (current_obj == nullptr) {
				Output::Warning("JSON_ERROR - Invalid path: {}", json_path);
				return std::string(json_data);
			}

			// Parse the value string and set the appropriate JSON type
			try {
				*current_obj = nlohmann::json::parse(value);
			}
			catch (const nlohmann::json::parse_error&) {
				// If parsing fails, treat it as a string value
				*current_obj = value;
			}

			return json_obj.dump();
		}
		catch (const std::exception& e) {
			Output::Warning("JSON_ERROR - {}", e.what());
			return std::string(json_data);
		}
	}

}

#endif // HAVE_NLOHMANN_JSON
