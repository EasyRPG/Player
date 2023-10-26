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

#ifndef EP_GAME_DESTINY_H
#define EP_GAME_DESTINY_H

#include <cstdint>
#include <sstream>
#include <vector>

#include "lcf/rpg/saveeventexecframe.h"


// Constants
constexpr const char* DESTINY_DLL = "Destiny.dll";


namespace Destiny {
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
}


/**
 * The Destiny Patch class
 */
class Game_Destiny {
public:
	// ctor and dtor
	Game_Destiny();
	~Game_Destiny();

public:
	// Member functions

	/**
	 * Load the Destiny Patch
	 */
	void Load();

	/**
	 * Initialize and apply the patch to the game interpreter
	 *
	 * @param dllVersion The Destiny.dll version
	 * @param language The DLL language. Usually for displaying errors and for decimal format.
	 * @param gameVersion The RPG_RT version
	 * @param extra Extra flags
	 * @param dwordSize Length of dowrd container
	 * @param floatSize Length of float container
	 * @param stringSize Length of string container
	 */
	void Initialize(
		uint32_t dllVersion,
		uint32_t language,
		uint32_t gameVersion,
		uint32_t extra,
		uint32_t dwordSize,
		uint32_t floatSize,
		uint32_t stringSize
	);

	/**
	 * Clear Destiny patch before close
	 */
	void Terminate();


	/*
	 * Make the DestinyScript code exctracting from the event script's comment command
	 *
	 * @param scriptData The event script data
	 * @return A full DestinyScript code extracted from the comment event command
	 */
	std::string MakeString(lcf::rpg::SaveEventExecFrame& scriptData);


	/*
	 * Evaluate DestinyScript code
	 *
	 * @param code The DestinyScript code to evaluate
	 * @return Whether evaluation is successful
	 */
	bool Interpret(const std::string& code);


private:
	// Member data

	// Destiny containers
	std::vector<int> _dwords;
	std::vector<double> _floats;
	std::vector<std::string> _strings;

	// Destiny data
	Destiny::Version _dllVersion;
	Destiny::Version _gameVersion;
	Destiny::Language _language;
	uint32_t _extra;

	// Settings
	bool _decimalComma;
};
#endif // !EP_GAME_DESTINY_H
