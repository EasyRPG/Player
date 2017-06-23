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

#ifndef _FILEFINDER_H_
#define _FILEFINDER_H_

// Headers
#include "system.h"

#include <string>
#include <cstdio>
#include <ios>
#include <unordered_map>
#include <vector>

#ifdef PSP2
#  include <psp2/types.h>
#  define Offset SceOff
#else
#  define Offset off_t
#endif

/**
 * FileFinder contains helper methods for finding case
 * insensitive files paths.
 */
namespace FileFinder {
	/**
	 * Adds RTP paths to the file finder
	 *
	 * @param warn_no_rtp_found Emits a warning on screen when no RTP was found.
	 */
	void InitRtpPaths(bool warn_no_rtp_found = true);

	/**
	 * Quits FileFinder.
	 */
	void Quit();

	/**
	 * { case lowered path, real path }
	 */
	typedef std::unordered_map<std::string, std::string> string_map;

	/**
	 * { case lowered directory name, non directory file list }
	 */
	typedef std::unordered_map<std::string, string_map> sub_members_type;

	struct DirectoryTree {
		std::string directory_path;
		string_map files, directories;
		sub_members_type sub_members;
	}; // struct DirectoryTree

	/**
	 * Finds an image file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param dir directory to check.
	 * @param name image file name to check.
	 * @return path to file.
	 */
	std::string FindImage(const std::string& dir, const std::string& name);

	/**
	 * Finds a file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param dir directory to check.
	 * @param name file name to check.
	 * @return path to file.
	 */
	std::string FindDefault(const std::string& dir, const std::string& name);

	/**
	 * Finds a file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param name the path and name.
	 * @return path to file.
	 */
	std::string FindDefault(const std::string& name);

	/**
	* Finds a file in a subdirectory of a custom directory tree.
	*
	* @param tree Project tree to search
	* @param dir directory to check
	* @param name the path and name
	* @return path to file.
	*/
	std::string FindDefault(const DirectoryTree& tree, const std::string& dir, const std::string& name);

	/**
	 * Finds a file from the root of a custom project tree.
	 *
	 * @param tree Project tree to search
	 * @param name the path and name
	 * @return path to file.
	 */
	std::string FindDefault(const DirectoryTree& tree, const std::string& name);

	/**
	 * Finds a music file.
	 * Searches through the Music folder of the current RPG Maker game and
	 * the RTP directories.
	 *
	 * @param name the music path and name.
	 * @return path to file.
	 */
	std::string FindMusic(const std::string& name);

	/**
	 * Finds a sound file.
	 * Searches through the Sound folder of the current RPG Maker game and
	 * the RTP directories.
	 *
	 * @param name the sound path and name.
	 * @return path to file.
	 */
	std::string FindSound(const std::string& name);

	/**
	 * Finds a font file.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param name the font name.
	 * @return path to file.
	 */
	std::string FindFont(const std::string& name);

	/**
	 * Opens a file specified by a UTF-8 string.
	 *
	 * @param name_utf8 filename in UTF-8.
	 * @param mode ("r", "w", etc).
	 * @return FILE*.
	 */
	FILE* fopenUTF8(const std::string& name_utf8, char const* mode);

	/**
	 * Creates stream from UTF-8 file name.
	 *
	 * @param name UTF-8 string file name.
	 * @param m stream mode.
	 * @return NULL if open failed.
	 */
	std::shared_ptr<std::fstream> openUTF8(const std::string& name, std::ios_base::openmode m);

	struct Directory {
		std::string base;
		string_map files;
		string_map directories;
	}; // struct Directory

	/**
	 * Checks whether passed file is directory.
	 * This function is case sensitve on some platform.
	 *
	 * @param file file to check.
	 * @return true if file is directory, otherwise false.
	 */
	bool IsDirectory(const std::string& file);

	/**
	 * Checks whether passed file exists.
	 * This function is case sensitve on some platform.
	 *
	 * @param file file to check
	 * @return true if file exists, otherwise false.
	 */
	bool Exists(const std::string& file);

	/**
	 * Appends name to directory.
	 *
	 * @param dir base directory.
	 * @param name file name to be appended to dir.
	 * @return combined path
	 */
	std::string MakePath(const std::string& dir, const std::string& name);

	/**
	 * Converts a path to the canonical equivalent.
	 * This generates a path that does not contain ".." or "." directories.
	 *
	 * @param path Path to normalize
	 * @param initial_deepness How deep the passed path is relative to the game root
	 * @return canonical path
	 */
	std::string MakeCanonical(const std::string& path, int initial_deepness);

	/**
	 * Splits a path in it's components.
	 *
	 * @param path Path to split
	 * @return Vector containing path components
	 */
	std::vector<std::string> SplitPath(const std::string& path);

	/**
	 * Returns the part of "path_in" that is inside "path_to".
	 * e.g. Input: /h/e/game, /h/e/game/Music/a.wav; Output: Music/a.wav
	 *
	 * @param path_to Path to a primary folder of path_in
	 * @param path_in Absolute path to the file, must start with path_to
	 *
	 * @return The part of path_in that is inside path_to. path_in when the path is not in path_to
	 */
	std::string GetPathInsidePath(const std::string& path_to, const std::string& path_in);

	/**
	 * Return the part of "path_in" that is inside the current games directory.
	 *
	 * @see GetPathInsidePath
	 * @param path_in An absolute path inside the game directory
	 * @return The part of path_in that is inside the game directory, path_in when it's not in the directory
	 */
	std::string GetPathInsideGamePath(const std::string& path_in);

	/**
	 * GetDirectoryMembers member listing mode.
	 */
	enum Mode {
		ALL, /**< list files and directory */
		FILES, /**< list only non-directory files */
		DIRECTORIES, /**< list only directories */
		RECURSIVE /**< list non-directory files recursively */
	};

	/**
	 * Lists directory members.
	 *
	 * @param dir directory to list members.
	 * @param m member listing mode.
	 * @param parent name of current relative folder (used if m is RECURSIVE)
	 * @return member list.
	 */
	Directory GetDirectoryMembers(const std::string& dir, Mode m = ALL, const std::string& parent = "");

	/**
	 * Sets the directory tree that is used for executing the current RPG Maker
	 * game.
	 *
	 * @param directory_tree Directory tree to use.
	 */
	void SetDirectoryTree(std::shared_ptr<DirectoryTree> directory_tree);

	/**
	 * Gets the directory tree that is used by the current game.
	 *
	 * @return directory tree
	 */
	const std::shared_ptr<DirectoryTree> GetDirectoryTree();
	const std::shared_ptr<DirectoryTree> CreateSaveDirectoryTree();
	std::shared_ptr<DirectoryTree> CreateDirectoryTree(std::string const& p, bool recursive = true);

	bool IsValidProject(DirectoryTree const& dir);
	bool IsRPG2kProject(DirectoryTree const& dir);
	bool IsEasyRpgProject(DirectoryTree const& dir);

	/**
	 * Checks whether the save directory contains any savegame with name
	 * SaveXX.lsd (XX from 00 to 15).
	 *
	 * @return If directory tree contains a savegame
	 */
	bool HasSavegame();

	/** Get the size of a file
	 *
	 * @param file the path to a file
	 * @return the filesize, or -1 on error
	 */
	Offset GetFileSize(const std::string& file);

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
