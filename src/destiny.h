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

#ifndef EP_DESTINY_H
#define EP_DESTINY_H

#include <cstdint>
#include <sstream>

#include "lcf/rpg/saveeventexecframe.h"


namespace Destiny {
	// Constants
	constexpr const char* DESTINY_DLL = "Destiny.dll";


	// Enums
	enum Language {
		DEUTSCH = 0,
		ENGLISH,
	};


	// Structs
	struct Version {
		uint16_t major;
		uint16_t minor;

		Version()
			: major(0), minor(0) {}
		Version(uint32_t version) {
			major = version >> 0x10;
			minor = version & 0xFFFF;
		}

		std::string toString() {
			std::stringstream ss;

			ss << major << '.' << minor;
			return ss.str();
		}
	};


	// Functions
	/**
	 * Load the Destiny module
	 */
	void Load();

	/**
	 * Initialize and apply the patch to the game interpreter
	 */
	void Initialize(
		uint32_t _dllVersion,
		uint32_t _language,
		uint32_t _gameVersion,
		uint32_t _extra,
		uint32_t _dwordSize,
		uint32_t _floatSize,
		uint32_t _stringSize
	);

	/**
	 * Clear Destiny patch before close
	 */
	void Terminate();


	// Interpret functions

	/*
	 * Make the DestinyScript code exctracting from the event script's comment command
	 */
	std::string MakeString(lcf::rpg::SaveEventExecFrame& scriptData);


	/*
	 * Evaluate DestinyScript code
	 */
	bool Interpret(const std::string& code);

}
#endif // !EP_DESTINY_H
