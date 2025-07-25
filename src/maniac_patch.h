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


#ifndef EP_MANIAC_PATCH
#define EP_MANIAC_PATCH

#include <array>
#include <cstdint>
#include <vector>
#include "filesystem_stream.h"
#include "span.h"

class Game_BaseInterpreterContext;

namespace ManiacPatch {
	int32_t ParseExpression(Span<const int32_t> op_codes, const Game_BaseInterpreterContext& interpreter);
	std::vector<int32_t> ParseExpressions(Span<const int32_t> op_codes, const Game_BaseInterpreterContext& interpreter);

	std::array<bool, 50> GetKeyRange();

	bool CheckString(std::string_view str_l, std::string_view str_r, int op, bool ignore_case);

	std::string_view GetLcfName(int data_type, int id, bool is_dynamic);
	std::string_view GetLcfDescription(int data_type, int id, bool is_dynamic);

	namespace GlobalSave {
		/**
		* Attempts to load Save.lgs from the save directory.
		* On success the data is stored in Main_Data::*_global variables.
		*
		* Is a no-op and returns true, when:
		* - Maniac Patch is not enabled
		* - The global save is already opened
		*
		* @return Whether loading was successful (on success the global save is considered opened)
		*/
		bool Load();

		/**
		* Attempts to load a Maniac Global Save file.
		* On success the data is stored in Main_Data::*_global variables.
		*
		* @param lgs_in Stream to read the global save from
		* @return Whether loading was successful
		*/
		bool Load(Filesystem_Stream::InputStream& lgs_in);

		/**
		* Saves to a Maniac Global Save file (Save.lgs in the save directory).
		* The save data is read from Main_Data::*_global variables.
		*
		* Is a no-op and returns true, when:
		* - Maniac Patch is not enabled
		* - The global save is not opened
		*
		* @param close_global_save When true marks the global save as closed on success
		* @return Whether saving was successful
		*/
		bool Save(bool close_global_save);

		/**
		* Saves to a Maniac Global Save file.
		* The save data is read from Main_Data::*_global variables.
		*
		* @param lgs_out Stream to write the global save to
		* @return Whether saving was successful
		*/
		bool Save(Filesystem_Stream::OutputStream& lgs_out);

		/**
		 * Resets the "open" flag for the global save.
		 * Open operations reload the file now.
		 */
		void Close();
	}
}

#endif
