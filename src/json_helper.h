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
#include <functional>

using json = nlohmann::ordered_json;

namespace Json_Helper {
/**
 * Parses a JSON string into a JSON object
 * @param json_data The JSON string to parse
 * @return The parsed JSON object, or empty if parsing failed
 */
std::optional<json> Parse(std::string_view json_data);

/**
 * Checks whether a passed JSON pointer is valid.
 * @param json_path JSON pointer to validate
 * @return true when fine, false if not
 */
bool CheckJsonPointer(std::string_view json_path);

/**
 * Gets a value from a JSON object using a JSON pointer path
 * @param json_obj The JSON object to get the value from
 * @param json_path The JSON pointer path to the value
 * @return The value as a string, or empty if missing
 */
std::string GetValue(json& json_obj, std::string_view json_path);

/**
 * Sets a value in a JSON object using a JSON pointer path
 * @param json_obj The JSON object to modify
 * @param json_path The JSON pointer path where to set the value
 * @param value The value to set (will be parsed as JSON if valid)
 * @return The modified JSON object as a string
 */
std::string SetValue(json& json_obj, std::string_view json_path, std::string_view value);

/**
 * Gets the length of an array or object at the specified path
 * @param json_obj The JSON object to get the length from
 * @param json_path The JSON pointer path to the array/object
 * @return The length or 0 is invalid or not an array/object
 */
size_t GetLength(const json& json_obj, std::string_view json_path);

/**
 * Gets all keys from a JSON object or indices from an array at the specified path
 * @param json_obj The JSON object to get the keys/indices from
 * @param json_path The JSON pointer path to the object or array
 * @return Vector of key names (for objects) or indices (for arrays), or empty if not an object/array
 */
std::vector<std::string> GetKeys(const json& json_obj, std::string_view json_path);

/**
 * Gets the type of value at the specified path
 * @param json_obj The JSON object to check
 * @param json_path The JSON pointer path to check
 * @return The type as a string ("object", "array", "string", "number", "boolean", "null"), or empty if missing
 */
std::string GetType(const json& json_obj, std::string_view json_path);

/**
 * Gets the full JSON pointer path to a specific value
 * @param json_obj The JSON object to search in
 * @param search_value The value to search for
 * @return The JSON pointer path to the value, or empty if not found
 */
std::string GetPath(const json& json_obj, const json& search_value);

/**
 * Returns a pretty-printed JSON string with custom indentation
 * @param json_obj The JSON object to format
 * @param indent Number of spaces for indentation (default: 2)
 * @return The formatted JSON string
 */
std::string PrettyPrint(const json& json_obj, int indent = 2);

/**
 * Removes a value at the specified path from a JSON object
 * @param json_obj The JSON object to modify
 * @param json_path The JSON pointer path to the value to remove
 * @return The modified JSON object as a string, or empty string if invalid
 */
std::string RemoveValue(json& json_obj, std::string_view json_path);

/**
 * Pushes a value to the end of an array at the specified path
 * @param json_obj The JSON object containing the array
 * @param json_path The JSON pointer path to the array
 * @param value The value to push (will be parsed as JSON if valid)
 * @return The modified JSON object as a string, or empty string if not an array
 */
std::string PushValue(json& json_obj, std::string_view json_path, std::string_view value);

/**
 * Removes and returns the last element from an array at the specified path
 * @param json_obj The JSON object containing the array
 * @param json_path The JSON pointer path to the array
 * @return Tuple of modified JSON object and popped value as a string. Tuple is empty string if not an array or empty
 */
std::tuple<std::string, std::string> PopValue(json& json_obj, std::string_view json_path);

/**
 * Checks if a key or array index exists at the specified path
 * @param json_obj The JSON object to check
 * @param json_path The JSON pointer path to check
 * @return true if exists, false if not
 */
bool Contains(const json& json_obj, std::string_view json_path);

} // namespace Json_Helper

#endif // HAVE_NLOHMANN_JSON
#endif // JSON_HELPER_H
