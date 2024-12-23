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


// Global constants
constexpr const char* DESTINY_DLL = "Destiny.dll";


namespace Destiny
{
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//			CONSTANTS
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	// Object flags
	constexpr const uint16_t OF_NONE		= 0x00;
	constexpr const uint16_t OF_CALL		= 0x01;
	constexpr const uint16_t OF_ARRAY		= 0x02;
	constexpr const uint16_t OF_CONSTANT	= 0x04;

	// Parameter flags
	constexpr const uint16_t PF_NONE = 0x0000;
	constexpr const uint16_t PF_BYTE = 0x0001;
	constexpr const uint16_t PF_WORD = 0x0002;
	constexpr const uint16_t PF_DWORD = 0x0003;
	constexpr const uint16_t PF_DOUBLE = 0x0004;
	constexpr const uint16_t PF_BOOL = 0x0005;
	constexpr const uint16_t PF_STRING = 0x0006;
	constexpr const uint16_t PF_EXTRA1 = 0x0100;
	constexpr const uint16_t PF_EXTRA2 = 0x0200;
	constexpr const uint16_t PF_EXTRA3 = 0x0400;
	constexpr const uint16_t PF_EXTRA4 = 0x0800;
	constexpr const uint16_t PF_FIXED = 0x1000;
	constexpr const uint16_t PF_OBJECT = 0x2000;
	constexpr const uint16_t PF_FUNCTION = 0x4000;
	constexpr const uint16_t PF_POINTER = 0x8000;

	// Sign flags
	constexpr const uint16_t VF_NONE		= 0x0000;
	constexpr const uint16_t VF_PLUS		= 0x0001;
	constexpr const uint16_t VF_MINUS		= 0x0002;
	constexpr const uint16_t VF_BINARYNOT	= 0x0003;
	constexpr const uint16_t VF_LOGICALNOT	= 0x0004;
	constexpr const uint16_t VF_INCREMENT	= 0x0100;
	constexpr const uint16_t VF_DECREMENT	= 0x0200;

	// Operator flags
	constexpr const uint16_t OF_LOGICALOR	= 0x0001;
	constexpr const uint16_t OF_LOGICALAND	= 0x0002;
	constexpr const uint16_t OF_EQUAL		= 0x0003;
	constexpr const uint16_t OF_UNEQUAL		= 0x0004;
	constexpr const uint16_t OF_ABOVE		= 0x0005;
	constexpr const uint16_t OF_ABOVEEQUAL	= 0x0006;
	constexpr const uint16_t OF_BELOW		= 0x0007;
	constexpr const uint16_t OF_BELOWEQUAL	= 0x0008;
	constexpr const uint16_t OF_CONCAT		= 0x0009;
	constexpr const uint16_t OF_ADD			= 0x000A;
	constexpr const uint16_t OF_SUBTRACT	= 0x000B;
	constexpr const uint16_t OF_MULTIPLY	= 0x000C;
	constexpr const uint16_t OF_DIVIDE		= 0x000D;
	constexpr const uint16_t OF_MODULO		= 0x000E;
	constexpr const uint16_t OF_SHIFTLEFT	= 0x000F;
	constexpr const uint16_t OF_SHIFTRIGHT	= 0x0010;
	constexpr const uint16_t OF_BINARYOR	= 0x0011;
	constexpr const uint16_t OF_BINARYXOR	= 0x0012;
	constexpr const uint16_t OF_BINARYAND	= 0x0013;
	constexpr const uint16_t OF_SET			= 0x0100;

	// Destiny flags
	constexpr const uint32_t DF_TRUECOLOR = 0b0001;		// Use AuroraSheets 32-bit
	constexpr const uint32_t DF_EVENTSYSTEM = 0b0010;	// The Destiny event system is used (not yet implemented)
	constexpr const uint32_t DF_HARMONY = 0b0100;		// The Harmony.dll has a DestinyInterface (not yet implemented)
	constexpr const uint32_t DF_PROTECT = 0b1000;		// Potentially unsafe functions are blocked (not yet implemented)

	// File access flags
	constexpr const uint8_t FILE_READ	= 0b0001;
	constexpr const uint8_t FILE_WRITE	= 0b0010;
	constexpr const uint8_t FILE_APPEND	= 0b0100;


	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//			ENUMS
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	enum Language
	{
		DEUTSCH = 0,
		ENGLISH,
	};

	enum InterpretFlag
	{
		IF_ERROR = 0,
		IF_COMMAND,
		IF_IF,
		IF_ELSEIF,
		IF_ELSE,
		IF_ENDIF,
		IF_DO,
		IF_LOOP,
		IF_WHILE,
		IF_UNTIL,
		IF_BREAK,
		IF_FOR,
		IF_NEXT,
		IF_SWITCH,
		IF_CASE,
		IF_DEFAULT,
		IF_ENDSWITCH,
		IF_CONTINUE,
		IF_PAUSE,
		IF_EXIT,
	};


	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//			STRUCTS
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	struct Version
	{
		uint16_t major;
		uint16_t minor;

		Version()
			: major(0), minor(0) {}
		Version(uint32_t version)
		{
			major = version >> 0x10;
			minor = version & 0xFFFF;
		}

		std::string toString() const
		{
			std::stringstream ss;

			ss << major << '.' << minor;
			return ss.str();
		}

		inline bool operator==(Version& other) const
		{
			return major == other.major && minor == other.minor;
		}

		inline bool operator!=(Version& other) const
		{
			return ! (*this == other);
		}

		inline bool operator>(Version& other) const
		{
			return minor == other.minor
				? major > other.major
				: minor > other.minor;
		}

		inline bool operator<(Version& other) const
		{
			return minor == other.minor
				? major < other.major
				: minor < other.minor;
		}

		inline bool operator>=(Version& other) const
		{
			return ! (*this < other);
		}

		inline bool operator<=(Version& other) const
		{
			return ! (*this > other);
		}
	};


	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//			CLASSES
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	namespace MainFunctions
	{
		class Interpreter
		{
		public:
			Interpreter();

		public:
			// Member functions

			/**
			 * Generates a DestinyScript code.
			 *
			 * @param frame		The event script data.
			 * @return			A DestinyScript code.
			 */
			const char* MakeString(lcf::rpg::SaveEventExecFrame& frame);

			/**
			 * Releases the DestinyScript code.
			 *
			 * @return
			 */
			void FreeString();

			/**
			 * Skip whitespaces from the DestinyScript code.
			 *
			 * @return
			 */
			void SkipWhiteSpace();

			/*
			 * Retreives the length of the next DestinyScript word.
			 *
			 * @returns The word found length.
			 */
			size_t GetWordLen();

			/*
			 * Evaluates the DestinyScript code.
			 *
			 * @returns The interpreter state flag.
			 */
			InterpretFlag Interpret();

			/**
			 * Loads the interpreter stack.
			 *
			 * @return
			 */
			void LoadInterpretStack();


			// Inline functions

			/**
			 * Cleans the interpreter data.
			 *
			 * @return
			 */
			inline void CleanUpData()
			{
				_breaks = 0U;
				_continues = 0U;
				_loopOperation = 0U;
				_loopOperationLevel = 0U;
			}

			/**
			 * Moves the DestinyScript pointer to forward.
			 *
			 * @return
			 */
			inline void ScriptNextChar()
			{
				++_scriptPtr;
			}

			/**
			 * Checkes whether interpreter reached the
			 * end of instruction symbol.
			 *
			 * @return
			 */
			inline bool IsEndOfLine() const
			{
				return _scriptPtr && *_scriptPtr == ';';
			}

			/**
			 * Checkes whether interpreter reached the
			 * end of the DestinyScript code.
			 *
			 * @return
			 */
			inline bool IsEndOfScript() const
			{
				return _scriptPtr && *_scriptPtr == '\0';
			}

		private:
			// Member data
			std::string _destinyScript;
			char* _scriptPtr;
			uint32_t _breaks;
			uint32_t _continues;
			uint32_t _loopOperation;
			uint32_t _loopOperationLevel;

			// Utility functions

			/**
			 * Skip all non-script objects.
			 *
			 * @return
			 */
			void SkipSpace();

			/**
			 * Read a line comment.
			 *
			 * @return Flag to finish spaces skipping.
			 */
			bool LineComment();

			/**
			 * Read a line block.
			 *
			 * @return Flag to finish spaces skipping.
			 */
			bool BlockComment();

			/**
			 * Check whether character is a whitespace.
			 *
			 * @return Flag of whitespace character.
			 */
			inline bool IsWhiteSpace(const char ch) const
			{
				return ch == ' ' ||
					ch == 0x09 ||		// Horizontal Tabulator (HT)
					ch == 0x0D ||		// Carriage return (CR)
					ch == 0x0A ||		// Line Feed (LF)
					ch == 0x0B;			// Vertical Tabulator (VT)
			}

			/**
			 * Check whether character is a part of a word.
			 *
			 * @return Flag of word character.
			 */
			inline bool IsWordChar(const char ch) const
			{
				return ch == '_' ||
					(ch >= '0' && ch <= '9') ||
					(ch >= 'A' && ch <= 'Z') ||
					(ch >= 'a' && ch <= 'z');
			}
		};
	}
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//		MAIN DESTINY CLASS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
 * The Destiny Patch class.
 */
class Game_Destiny
{
public:
	// ctor and dtor
	Game_Destiny();
	~Game_Destiny();

public:
	// Member functions

	/**
	 * Load the Destiny Patch.
	 *
	 * @return
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
	 * Clear Destiny patch before close.
	 *
	 * @return
	 */
	void Terminate();

	/**
	 * Call the Destiny Interpreter and run the received code.
	 *
	 * @param frame		The event script data.
	 * @return			Whether evaluation is successful.
	 */
	bool Main(lcf::rpg::SaveEventExecFrame& frame);


	// Inline functions

	/**
	 * Retrieves the Destiny interpreter.
	 *
	 * @returns The Destiny interpreter.
	 */
	inline Destiny::MainFunctions::Interpreter& Interpreter()
	{
		return _interpreter;
	}

private:
	// Member data

	// Destiny main functions
	Destiny::MainFunctions::Interpreter _interpreter;

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
	uint32_t _trueColor;
	bool _decimalComma;
	bool _rm2k3;
	bool _protect;

	// Utility functions

	/**
	 * Evaluate the extra flags.
	 *
	 * @return
	 */
	void EvaluateExtraFlags();

	/**
	 * Check the version information.
	 *
	 * @return
	 */
	void CheckVersionInfo();
};
#endif // !EP_GAME_DESTINY_H
