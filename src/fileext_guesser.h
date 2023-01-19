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

#ifndef EP_FILEEXTGUESSER_H
#define EP_FILEEXTGUESSER_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include "filesystem.h"

class Meta;

/**
 * FileExtGuesser contains helper methods for guessing the extensions used on non-standard RPG Projects.
 */
namespace FileExtGuesser {

	// Bookkeeping structures for use with GuessAndAddLmuExtension()
	struct RPG2KFileExtRemap {
		/**
		 * Construct a filename from a given prefix and suffix.
		 * Performs extension substitution based on the values stored in extMap
		 *
		 * @param prefix The prefix (e.g., 'Map0001')
		 * @param suffix The suffix (e.g., 'lmu')
		 * @return The joined filename (e.g., 'Map0001.lmu', OR 'Map0001.xyz')
		 */
		std::string MakeFilename(StringView prefix, StringView suffix);

		std::unordered_map<std::string, std::string> extMap;
	};

	/**
	 * Attempts to determine the LMU extension for non-standard projects.
	 *
	 * @param fs The filesystem of the project in question
	 * @param meta The meta object, which can be used to directly specify the extension
	 * @param mapping The resultant mapping, if any, is stored in this lookup.
	 */
	void GuessAndAddLmuExtension(const FilesystemView& fs, Meta const& meta, RPG2KFileExtRemap& mapping);

	// Bookkeeping structure for use with GetRPG2kProjectWithRenames()
	struct RPG2KNonStandardFilenameGuesser {
		struct ExtAndSize {
			ExtAndSize(std::string fname = "", std::string ext = "", int64_t sz = 0) : fname(std::move(fname)), ext(std::move(ext)), sz(sz) {}
			std::string fname;
			std::string ext;
			int64_t sz;
		};

		// This contains the LMT and LDB files, in no particular order.
		std::pair<ExtAndSize,ExtAndSize> rpgRTs;

		/**
		 * Is this struct 'empty' --i.e., does it contain no useful information?
		 *
		 * @return true if both rpgTRs's 'ext' properties are empty; false otherwise
		 */
		bool Empty() const;

		/**
		 * Perform a series of heuristical guesses to determine what the LDB/LMT extensions are.
		 *
		 * @param meta A Meta object loaded from the INI file
		 * @return A mapping for the LDB and LMT extensions (may be empty if no guess could be made).
		 */
		RPG2KFileExtRemap guessExtensions(Meta& meta);
	};

	/**
	 * Scans a directory tree and tries to identify the LMT/LDB files, but with non-standard extensions.
	 *
	 * @param fs The filesystem of the project in question
	 * @return An object that contains the candidates (check with .Empty())
	 */
	RPG2KNonStandardFilenameGuesser GetRPG2kProjectWithRenames(const FilesystemView& fs);

}

#endif

