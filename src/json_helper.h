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

#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include "system.h"

#ifdef HAVE_NLOHMANN_JSON

#include <optional>
#include <string_view>
#include <nlohmann/json.hpp>

namespace Json_Helper {
	std::optional<nlohmann::json> Parse(std::string_view json_data);
	std::optional<std::string> GetValue(nlohmann::json& json_obj, std::string_view json_path);
	std::string SetValue(nlohmann::json& json_obj, std::string_view json_path, std::string_view value);
}

#endif // HAVE_NLOHMANN_JSON
#endif // JSON_HELPER_H
