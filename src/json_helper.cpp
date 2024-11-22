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
#include "string_view.h"

using json = nlohmann::json;

namespace {
	std::string GetValueAsString(const json& json_obj) {
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
}

namespace Json_Helper {
	std::optional<nlohmann::json> Parse(std::string_view json_data) {
		json json_obj = json::parse(json_data, nullptr, false);
		if (json_obj.is_discarded()) {
    		return {};
		}

		return json_obj;
	}

	std::optional<std::string> GetValue(nlohmann::json& json_obj, std::string_view json_path) {
		json::json_pointer ptr((std::string(json_path)));

		if (ptr.empty()) {
			Output::Warning("JSON: Bad json pointer {}", json_path);
			return {};
		}

		if (!json_obj.contains(ptr)) {
			return "";
		}

		return GetValueAsString(json_obj[ptr]);
	}

	std::string SetValue(nlohmann::json& json_obj, std::string_view json_path, std::string_view value) {
		json::json_pointer ptr((std::string(json_path)));

		if (ptr.empty()) {
			Output::Warning("JSON: Bad json pointer {}", json_path);
			return {};
		}

		json obj_value = json::parse(value, nullptr, false);

		if (obj_value.is_discarded()) {
			// If parsing fails, treat it as a string value
			json_obj[ptr] = std::string(value);
		} else {
			json_obj[ptr] = obj_value;
		}

		return json_obj.dump();
	}
}

#endif // HAVE_NLOHMANN_JSON
