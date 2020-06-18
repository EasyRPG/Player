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
#include "lcf/inireader.h"


/**
 * The Meta class loads from a file (easyrpg.ini) and contains 
 * additional information on a per-game basis.
 * Every public function in this class is safe to call and
 * returns sensible defaults, even if the INI file passed
 * in to the constructor is missing or invalid.
 */

class Meta {
public:
	/**
	 * Construct a Meta object by parsing a file
	 * @param meta_file The path to the ini file to load
	 */
	Meta(const std::string& meta_file);

	/**
	 * Retrieves the map used for pivoting between multi-game save files
	 * All save files listed will match the given pivot map ID (i.e., "last saved here")
	 * @return the pivot map ID, or 0 (on error) for "no map restriction"
	 */
	int GetPivotMap() const;

	/**
	 * Retrieves the canonical name of the parent game (i.e., the prequel) for multi-game save files.
	 * @return the parent game name
	 */
	std::string GetParentGame() const;

	/** Small struct used for loading a list of potential import saves piecemeal */
	struct FileItem {
		std::string short_path;
		std::string full_path;
		int file_id = 0;
	};

	/**
	 * Retrieves a vector of paths of child games (../ from the current game) that may be 
	 * considered when searching for multi-game save files. These should be passed to SearchImportPaths()
	 * @param parent_tree the tree for the parent folder of the current game (../)
	 * @return vector of paths of child game folders, including that of the current game
	 */
	std::vector<std::string> GetImportChildPaths(const FileFinder::DirectoryTree& parent_tree) const;

	/**
	 * Given a parent/child game, retrieves a vector of save files that are considered for multi-game importing.
	 * @param parent_tree the tree for the parent folder of the current game (../)
	 * @param child_path the path of the child relative to parent_tree
	 * @return vector of FileItems; one for each valid save file on this child_path
	 */
	std::vector<FileItem> SearchImportPaths(const FileFinder::DirectoryTree& parent_tree, const std::string& child_path) const;

	/**
	 * Is multi-game save importing enabled? If so, the title screen should show an Import option.
	 * @return true if the meta INI file contains a parentGame for this game
	 */
	bool IsImportEnabled() const;

	/**
	 * Retrieve the Help window text for Scene_Import
	 * @return the INI-defined value, or the defualt value for this vocabulary term
	 */
	std::string GetExVocabImportSaveHelpText() const;

	/**
	 * Retrieve the menu item text for Scene_Ttile importing
	 * @return the INI-defined value, or the defualt value for this vocabulary term
	 */
	std::string GetExVocabImportSaveTitleText() const;
	

private:
	/**
	 * Retrieve an extended vocabulary term (something defined in easyrpg.ini)
	 * A list of terms and default values can be found in meta.cpp
	 * @param term The keyword to search for
	 * @param def_value The default value to return if the ini contains no override.
	 * @return the INI-defined value, or the defualt value for this vocabulary term
	 */
	std::string GetExVocab(const std::string& term, const std::string& def_value) const;
	
	/**
	 * Heuristically tries to guess the canonical name of this game,
	 *  and stores it locally in this Meta object.
	 */
	void IdentifyCanonName();

	/**
	 * Internal function called by SearchImportPaths
	 * @param parent_tree the tree for the parent folder of the current game (../)
	 * @param child_path the path of the child relative to parent_tree
	 * @param parent_game_name canonical name of the parent game
	 * @param pivot_map_id the id of the map used to pivot between the prequel and the current game
	 * @return vector of FileItems; one for each valid save file on this child_path
	 */
	std::vector<FileItem> BuildImportCandidateList(const FileFinder::DirectoryTree& parent_tree, const std::string& child_path, const std::string& parent_game_name, int pivot_map_id) const;

	/**
	 * Was the INI file passed to the constructor invalid or empty?
	 * @return true if this class should be considered "empty"
	 */
	bool Empty() const;

	/** Root of the lcf::INIReader */
	std::unique_ptr<lcf::INIReader> ini;

	/**
	 * Current canonical lookup string in easyrpg.ini
	 * Format is "Crc32LMT/Crc32LDB", where
	 * Crc32LDB can be "*" to represent "any".
	 */
	std::string canon_ini_lookup;
};

#endif  // EP_META_H
