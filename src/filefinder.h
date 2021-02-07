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

#ifndef EP_FILEFINDER_H
#define EP_FILEFINDER_H

// Headers
#include "system.h"
#include "filesystem_stream.h"
#include "string_view.h"
#include "directory_tree.h"

#include <string>
#include <cstdio>
#include <ios>
#include <istream>
#include <unordered_map>
#include <vector>

/**
 * FileFinder contains helper methods for finding case
 * insensitive files paths.
 */
namespace FileFinder {
	/**
	 * Quits FileFinder.
	 */
	void Quit();

	/**
	 * Finds an image file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param dir directory to check.
	 * @param name image file name to check.
	 * @return path to file.
	 */
	std::string FindImage(StringView dir, StringView name);

	/**
	 * Finds a file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param dir directory to check.
	 * @param name file name to check.
	 * @return path to file.
	 */
	std::string FindDefault(StringView dir, StringView name);

	/**
	 * Finds a file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param name the path and name.
	 * @return path to file.
	 */
	std::string FindDefault(StringView name);

	/**
	 * Finds a music file.
	 * Searches through the Music folder of the current RPG Maker game and
	 * the RTP directories.
	 *
	 * @param name the music path and name.
	 * @return path to file.
	 */
	std::string FindMusic(StringView name);

	/**
	 * Finds a sound file.
	 * Searches through the Sound folder of the current RPG Maker game and
	 * the RTP directories.
	 *
	 * @param name the sound path and name.
	 * @return path to file.
	 */
	std::string FindSound(StringView name);

	/**
	 * Finds a font file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param name the font name.
	 * @return path to file.
	 */
	std::string FindFont(StringView name);

	/**
	* Creates stream from UTF-8 file name.
	*
	* @param name UTF-8 string file name.
	* @param m stream mode.
	* @return NULL if open failed.
	*/
	Filesystem_Stream::InputStream OpenInputStream(const std::string& name,
			std::ios_base::openmode m = std::ios_base::in | std::ios_base::binary);

	/**
	* Creates stream from UTF-8 file name.
	*
	* @param name UTF-8 string file name.
	* @param m stream mode.
	* @return NULL if open failed.
	*/
	Filesystem_Stream::OutputStream OpenOutputStream(const std::string& name,
			std::ios_base::openmode m = std::ios_base::out | std::ios_base::binary);

	/**
	 * Checks whether passed file is directory.
	 * This function is case sensitve on some platform.
	 *
	 * @param file file to check.
	 * @param follow_symlinks if true, follow symlinks and report about the target.
	 * @return true if file is directory, otherwise false.
	 */
	bool IsDirectory(StringView file, bool follow_symlinks);

	/**
	 * Checks whether passed file exists.
	 * This function is case sensitve on some platform.
	 *
	 * @param file file to check
	 * @return true if file exists, otherwise false.
	 */
	bool Exists(StringView file);

	/**
	 * Appends name to directory.
	 *
	 * @param dir base directory.
	 * @param name file name to be appended to dir.
	 * @return combined path
	 */
	std::string MakePath(StringView dir, StringView name);

	/**
	 * Converts a path to the canonical equivalent.
	 * This generates a path that does not contain ".." or "." directories.
	 *
	 * @param path Path to normalize
	 * @param initial_deepness How deep the passed path is relative to the game root
	 * @return canonical path
	 */
	std::string MakeCanonical(StringView path, int initial_deepness);

	/**
	 * Splits a path in it's components.
	 *
	 * @param path Path to split
	 * @return Vector containing path components
	 */
	std::vector<std::string> SplitPath(StringView path);

	/**
	 * Splits a path into path and filename.
	 *
	 * @param path Path to split
	 * @return Pair containing dir and name
	 */
	std::pair<std::string, std::string> GetPathAndFilename(StringView path);

	/**
	 * Converts all path delimiters to forward slash (/)
	 *
	 * @param path to convert
	 */
	void ConvertPathDelimiters(std::string& path);

	/**
	 * Returns the part of "path_in" that is inside "path_to".
	 * e.g. Input: /h/e/game, /h/e/game/Music/a.wav; Output: Music/a.wav
	 *
	 * @param path_to Path to a primary folder of path_in
	 * @param path_in Absolute path to the file, must start with path_to
	 *
	 * @return The part of path_in that is inside path_to. path_in when the path is not in path_to
	 */
	std::string GetPathInsidePath(StringView path_to, StringView path_in);

	/**
	 * Return the part of "path_in" that is inside the current games directory.
	 *
	 * @see GetPathInsidePath
	 * @param path_in An absolute path inside the game directory
	 * @return The part of path_in that is inside the game directory, path_in when it's not in the directory
	 */
	std::string GetPathInsideGamePath(StringView path_in);

	/**
	 * Sets the directory tree that is used for executing the current RPG Maker
	 * game.
	 *
	 * @param directory_tree Directory tree to use.
	 */
	void SetDirectoryTree(std::unique_ptr<DirectoryTree> directory_tree);

	/**
	 * Gets the directory tree that is used by the current game.
	 *
	 * @return directory tree
	 */
	DirectoryTreeView GetDirectoryTree();

	/** @return A new directory tree that is rooted at the save directory or nullptr when not readable */
	std::unique_ptr<DirectoryTree> CreateSaveDirectoryTree();

	/**
	 * @param p root directory of the tree
	 * @return New directory tree or nullptr when p is not readable
	 */
	std::unique_ptr<DirectoryTree> CreateDirectoryTree(std::string p);

	/**
	 * @param p tree Tree to check
	 * @return Whether the tree contains a valid RPG2k(3) or EasyRPG project
	 */
	bool IsValidProject(const DirectoryTreeView& tree);

	/**
	 * @param p tree Tree to check
	 * @return Whether the tree contains a valid RPG2k(3) project
	 */
	bool IsRPG2kProject(const DirectoryTreeView& tree);

	/**
	 * @param p tree Tree to check
	 * @return Whether the tree contains a valid EasyRPG project
	 */
	bool IsEasyRpgProject(const DirectoryTreeView& tree);

	/**
	 * Determines if the directory in question represents an RPG2k project with non-standard
	 *   database, map tree, or map file names.
	 *
	 * @param tree The directory tree in question
	 * @return true if this is likely an RPG2k project; false otherwise
	 */
	bool IsRPG2kProjectWithRenames(const DirectoryTreeView& tree);

	/**
	 * Checks whether the save directory contains any savegame with name
	 * SaveXX.lsd (XX from 00 to 15).
	 *
	 * @return If directory tree contains a savegame
	 */
	bool HasSavegame();

	/** @returns Amount of savegames in the save directory */
	int GetSavegames();

	/**
	 * Get the size of a file
	 *
	 * @param file the path to a file
	 * @return the filesize, or -1 on error
	 */
	int64_t GetFileSize(StringView file);

	/**
	 * Known file sizes
	 */
	enum KnownFileSize {
		OFFICIAL_HARMONY_DLL = 473600,
	};

	/**
	 * Checks whether the game is created with RPG2k >= 1.50 or RPG2k3 >= 1.05.
	 *
	 * @return true if RPG2k >= 1.50 or RPG2k3 >= 1.05, otherwise false.
	 */
	bool IsMajorUpdatedTree();

	/** RPG_RT.exe file size thresholds
         *
         * 2k v1.51 (Japanese)    : 746496
         * 2k v1.50 (Japanese)    : 745984
         *  -- threshold (2k) --  : 735000
         * 2k v1.10 (Japanese)    : 726016
         *
         * 2k3 v1.09a (Japanese)  : 950784
         * 2k3 v1.06 (Japanese)   : 949248
         * 2k3 v1.05 (Japanese)   : unknown
         *  -- threshold (2k3) -- : 927000
         * 2k3 v1.04 (Japanese)   : 913408
         */
	enum RpgrtMajorUpdateThreshold {
		RPG2K = 735000,
		RPG2K3 = 927000,
	};
} // namespace FileFinder

#endif
