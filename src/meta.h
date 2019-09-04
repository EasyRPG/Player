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

#ifndef EP_META_H
#define EP_META_H

// Headers
#include <map>
#include <memory>
#include <string>
#include "filefinder.h"
#include "inireader.h"


/**
 * The Meta class loads from a file (easyrpg.ini) and contains 
 * additional information on a per-game basis.
 */

class Meta {
public:
	// @ini_file - The path to the ini file to load
	Meta(const std::string& meta_file);

	// Map to use for pivoting between multi-game
	// Returns 0 on error (i.e., "no map restriction")
	int GetPivotMap() const;

	// Parent game ID
	std::string GetParentGame() const;

	// Used for loading a list of potential import saves piecemeal
	struct FileItem {
		FileItem() : fileId(0) {}

		std::string shortPath;
		std::string fullPath;
		int fileId;
	};
	std::vector<std::string> GetImportChildPaths(const FileFinder::DirectoryTree& parentTree) const;
	std::vector<FileItem> SearchImportPaths(const FileFinder::DirectoryTree& parentTree, const std::string& child_path) const;

	// Is multi-game importing enabled?
	bool IsImportEnabled() const;

	// Helpers for specific extended vocabulary items
	std::string GetExVocabImportSaveHelpText() const;
	std::string GetExVocabImportSaveTitleText() const;
	

private:
	// Retrieve an extended vocabulary item
	// Look up the string value of the default.
	std::string GetExVocab(const std::string& term, const std::string& defValue) const;
	
	// Heuristically tries to guess the canonical name of this game,
	//  and stores it locally in this Meta object.
	bool IdentifyCanonName();

	std::vector<FileItem> BuildImportCandidateList(const FileFinder::DirectoryTree& parentTree, const std::string& child_path, const std::string& parentGameName, int pivotMapId) const;

	// Returns true if no metadata (file) exists
	bool Empty() const;

	// Root of the INIReader
	std::unique_ptr<INIReader> ini;

	// Current canonical lookup string in easyrpg.ini
	// Can be "Crc32LMT/Crc32LDB" or "Crc32LMT/*"
	std::string canon_ini_lookup;
};

#endif  // EP_META_H
