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
#include "filesystem.h"
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

	/** @return A filesystem handle for arbitrary file access inside the host filesystem */
	FilesystemView Root();

	/** @return A filesystem handle for file access inside the game directory */
	FilesystemView Game();

	/**
	 * Sets the game filesystem.
	 *
	 * @param filesystem Game filesystem to use.
	 */
	void SetGameFilesystem(FilesystemView filesystem);

	/**
	 * A filesystem handle for file access inside the save directory.
	 * Do not expect that this is the same as the game filesystem.
	 * The path is based on Main_Data::SavePath but could be redirected when
	 * the path is not writable because the filesystem does not support it.
	 *
	 * @return Save filesystem handle
	 */
	FilesystemView Save();

	/**
	 * Sets the save filesystem.
	 *
	 * @param filesystem Save filesystem to use.
	 */
	void SetSaveFilesystem(FilesystemView filesystem);

	/**
	 * Finds an image file in the current RPG Maker game.
	 *
	 * @param dir directory to check.
	 * @param name image file name to check.
	 * @return path to file.
	 */
	std::string FindImage(StringView dir, StringView name);

	/**
	 * Finds a music file in the current RPG Maker game.
	 *
	 * @param name the music path and name.
	 * @return path to file.
	 */
	std::string FindMusic(StringView name);

	/**
	 * Finds a sound file in the current RPG Maker game.
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
	 * Finds a text file in the current RPG Maker game.
	 *
	 * @param name the text path and name.
	 * @return path to file.
	 */
	std::string FindText(StringView name);

	/**
	 * Finds an image file and opens a file handle to it.
	 * Searches through the current RPG Maker game and the RTP directories.
	 *
	 * @param dir directory to check.
	 * @param name image file name to check.
	 * @return read handle on success or invalid handle if not found
	 */
	Filesystem_Stream::InputStream OpenImage(StringView dir, StringView name);

	/**
	 * Finds a music file and opens a file handle to it.
	 * Searches through the Music folder of the current RPG Maker game and
	 * the RTP directories.
	 *
	 * @param name the music path and name.
	 * @return read handle on success or invalid handle if not found
	 */
	Filesystem_Stream::InputStream OpenMusic(StringView name);

	/**
	 * Finds a sound file and opens a file handle to it.
	 * Searches through the Sound folder of the current RPG Maker game and
	 * the RTP directories.
	 *
	 * @param name the sound path and name.
	 * @return read handle on success or invalid handle if not found
	 */
	Filesystem_Stream::InputStream OpenSound(StringView name);

	/**
	 * Finds a font file and opens a file handle to it.
	 * Searches through the Font folder of the current RPG Maker game.
	 *
	 * @param name the font path and name.
	 * @return read handle on success or invalid handle if not found
	 */
	Filesystem_Stream::InputStream OpenFont(StringView name);

	/**
	 * Finds a textt file and opens a file handle to it.
	 * Searches through the Text folder of the current RPG Maker game.
	 *
	 * @param name the text path and name.
	 * @return read handle on success or invalid handle if not found
	 */
	Filesystem_Stream::InputStream OpenText(StringView name);

	/**
	 * Appends name to directory.
	 *
	 * @param dir base directory.
	 * @param name file name to be appended to dir.
	 * @return combined path
	 */
	std::string MakePath(StringView dir, StringView name);

	/**
	 * Creates a path out of multiple components
	 *
	 * @param path Path to join
	 */
	template<typename T>
	std::string MakePath(lcf::Span<T> components);

	/**
	 * Converts a path to the canonical equivalent.
	 * This generates a path that does not contain ".." or "." directories.
	 *
	 * @param path Path to normalize
	 * @param initial_deepness How deep the passed path is relative to the game root
	 * @return canonical path
	 */
	std::string MakeCanonical(StringView path, int initial_deepness = -1);

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
	 * Converts all path delimiters to the platform delimiters.
	 * \ on Windows, / on others.
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
	 * @param p tree Tree to check
	 * @return Whether the tree contains a valid RPG2k(3) or EasyRPG project
	 */
	bool IsValidProject(const FilesystemView& fs);

	/**
	 * @param p tree Tree to check
	 * @return Whether the tree contains a valid RPG2k(3) project
	 */
	bool IsRPG2kProject(const FilesystemView& fs);

	/**
	 * @param p tree Tree to check
	 * @return Whether the tree contains a valid EasyRPG project
	 */
	bool IsEasyRpgProject(const FilesystemView& fs);

	/**
	 * Determines if the directory in question represents an RPG2k project with non-standard
	 *   database, map tree, or map file names.
	 *
	 * @param tree The directory tree in question
	 * @return true if this is likely an RPG2k project; false otherwise
	 */
	bool IsRPG2kProjectWithRenames(const FilesystemView& fs);

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

	/**
	 * Utility function for getting a complete filesystem path.
	 *
	 * @param fs Filesystem to use
	 * @return complete path
	 */
	std::string GetFullFilesystemPath(FilesystemView fs);

	/**
	 * Utility function for debug printing of a filesystem path.
	 *
	 * @param fs Filesystem to use
	 */
	void DumpFilesystem(FilesystemView fs);
} // namespace FileFinder

template<typename T>
std::string FileFinder::MakePath(lcf::Span<T> components) {
	std::string path;
	for (auto& c: components) {
		path = MakePath(path, c);
	}
	return path;
}

#endif
