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
#include <nlohmann/json.hpp>
#include <sstream>
#include <unordered_map>
#include <charconv>
#include "string_view.h"

namespace {

	std::string GetValueAsString(const json& json_obj) {
		if (json_obj.is_null()) {
			return "null";
		}
		if (json_obj.is_string()) {
			return json_obj.get<std::string>();
		}
		if (json_obj.is_number_integer()) {
			return std::to_string(json_obj.get<int64_t>());
		}
		if (json_obj.is_number_float()) {
			return std::to_string(json_obj.get<double>());
		}
		if (json_obj.is_boolean()) {
			return json_obj.get<bool>() ? "true" : "false";
		}
		return json_obj.dump();
	}

} // namespace

namespace Json_Helper {

	std::optional<json> Parse(std::string_view json_data) {
		json json_obj = json::parse(json_data, nullptr, false);
		if (json_obj.is_discarded()) {
			return {};
		}
		return json_obj;
	}

	bool CheckJsonPointer(std::string_view json_path) {
		if (json_path.empty()) {
			Output::Warning("JSON: Empty json pointer at: {}", json_path);
			return false;
		}

		if (json_path.front() != '/') {
			Output::Warning("JSON: Json pointer must start with /: {}", json_path);
			return false;
		}

		return true;
	}

	std::string GetValue(json& json_obj, std::string_view json_path) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);

		if (!json_obj.contains(ptr)) {
			return {};
		}

		const json& value = json_obj[ptr];
		auto val = GetValueAsString(value);
		return val;
	}


	std::string SetValue(json& json_obj, std::string_view json_path, std::string_view value) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);

		json obj_value = json::parse(value, nullptr, false);
		if (obj_value.is_discarded()) {
			// If parsing fails, treat it as a string value
			json_obj[ptr] = std::string(value);
		}
		else {
			json_obj[ptr] = obj_value;
		}

		return json_obj.dump();
	}

	size_t GetLength(const json& json_obj, std::string_view json_path) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);

		if (!json_obj.contains(ptr)) {
			return 0;
		}

		const json& value = json_obj[ptr];
		if (!value.is_array() && !value.is_object()) {
			return 0;
		}

		return value.size();
	}

	std::vector<std::string> GetKeys(const json& json_obj, std::string_view json_path) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);
		if (!json_obj.contains(ptr)) {
			return {};
		}

		const json& value = json_obj[ptr];

		std::vector<std::string> keys;

		if (value.is_object()) {
			for (const auto& item : value.items()) {
				keys.push_back(item.key());
			}
		}
		else if (value.is_array()) {
			for (size_t i = 0; i < value.size(); ++i) {
				keys.push_back(std::to_string(i));
			}
		}
		return keys;
	}

	std::string GetType(const json& json_obj, std::string_view json_path) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);
		if (!json_obj.contains(ptr)) {
			return {};
		}

		const json& value = json_obj[ptr];

		if (value.is_object()) return std::string("object");
		if (value.is_array()) return std::string("array");
		if (value.is_string()) return std::string("string");
		if (value.is_number()) return std::string("number");
		if (value.is_boolean()) return std::string("boolean");
		if (value.is_null()) return std::string("null");
		return std::string("unknown");
	}

	std::string GetPath(const json& json_obj, const json& search_value) {
		std::function<std::string(const json&, const json&, const std::string&)> find_path;

		find_path = [&find_path](const json& obj, const json& target, const std::string& current_path) -> std::string {
			if (obj == target) {
				return current_path;
			}

			if (obj.is_object()) {
				for (const auto& item : obj.items()) {
					auto path = find_path(item.value(), target, current_path + "/" + item.key());
					if (!path.empty()) return path;
				}
			}
			else if (obj.is_array()) {
				for (size_t i = 0; i < obj.size(); ++i) {
					auto path = find_path(obj[i], target, current_path + "/" + std::to_string(i));
					if (!path.empty()) return path;
				}
			}
			return {};
			};

		return find_path(json_obj, search_value, "");
	}

	std::string PrettyPrint(const json& json_obj, int indent) {
		return json_obj.dump(std::max(0, indent));
	}

	std::string RemoveValue(json& json_obj, std::string_view json_path) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);

		if (!json_obj.contains(ptr)) {
			return {};
		}

		// Get parent path and key/index to remove
		auto parent_ptr = ptr.parent_pointer();

		json& parent = json_obj[parent_ptr];
		json_path.remove_prefix(parent_ptr.to_string().size() + 1);

		if (parent.is_object()) {
			parent.erase(std::string(json_path));
		}
		else if (parent.is_array()) {
			// Check if key is a valid positive number
			unsigned index;
			auto [ptr, ec] = std::from_chars(json_path.data(), json_path.data() + json_path.size(), index);
			if (ec == std::errc()) {
				if (index < parent.size()) {
					parent.erase(index);
				}
			} else {
				Output::Warning("JSON: Invalid array index at: {}", json_path);
				return {};
			}
		}

		return json_obj.dump();
	}

	std::string PushValue(json& json_obj, std::string_view json_path, std::string_view value) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);

		if (!json_obj.contains(ptr)) {
			return {};
		}

		json& array = json_obj[ptr];
		if (!array.is_array()) {
			Output::Warning("JSON: Path does not point to an array: {}", json_path);
			return {};
		}

		json obj_value = json::parse(value, nullptr, false);
		if (obj_value.is_discarded()) {
			// If parsing fails, treat it as a string value
			array.push_back(std::string(value));
		}
		else {
			array.push_back(obj_value);
		}

		return json_obj.dump();
	}

	std::tuple<std::string, std::string> PopValue(json& json_obj, std::string_view json_path) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);

		if (!json_obj.contains(ptr)) {
			return {};
		}

		json& array = json_obj[ptr];
		if (!array.is_array() || array.empty()) {
			Output::Warning("JSON: Path does not point to a non-empty array: {}", json_path);
			return {};
		}

		json popped = array.back();
		array.erase(array.size() - 1);

		return {json_obj.dump(), GetValueAsString(popped)};
	}

	bool Contains(const json& json_obj, std::string_view json_path) {
		std::string path_str = std::string(json_path);
		json::json_pointer ptr(path_str);

		return json_obj.contains(ptr);
	}

} // namespace Json_Helper

#endif // HAVE_NLOHMANN_JSON
