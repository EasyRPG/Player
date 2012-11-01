/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _FILEFINDER_H_
#define _FILEFINDER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "system.h"
#include <boost/container/flat_map.hpp>

////////////////////////////////////////////////////////////
/// FileFinder contains helper methods for finding case
/// insensitive files paths.
////////////////////////////////////////////////////////////
namespace FileFinder {
	////////////////////////////////////////////////////////
	/// Initialize FileFinder.
	////////////////////////////////////////////////////////
	void Init();

	////////////////////////////////////////////////////////
	/// Next Init step that is performed after parsing
	/// the database file. The rtp paths should be added
	/// to the FileFinder here.
	////////////////////////////////////////////////////////
	void InitRtpPaths();

	////////////////////////////////////////////////////////
	/// Quit FileFinder.
	////////////////////////////////////////////////////////
	void Quit();

	///////////////////////////////////////////////////////
	/// Find an image file.
	/// @param name : the image path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindImage(const std::string& dir, const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a file.
	/// @param name : the path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindDefault(const std::string& dir, const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a file.
	/// @param name : the path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindDefault(const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a music file.
	/// @param name : the music path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindMusic(const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a sound file.
	/// @param name : the sound path and name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindSound(const std::string& name);

	///////////////////////////////////////////////////////
	/// Find a font file.
	/// @param name : the font name
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string FindFont(const std::string& name);

	///////////////////////////////////////////////////////
	/// Get a default font.
	/// @return path to file
	///////////////////////////////////////////////////////
	std::string DefaultFont();

	///////////////////////////////////////////////////////
	/// Open a file specified by a UTF-8 string
	/// @param : filename in UTF-8
	/// @param : mode ("r", "w", etc)
	/// @return: FILE*
	///////////////////////////////////////////////////////
	FILE* fopenUTF8(const std::string& name_utf8, char const* mode);

	typedef boost::container::flat_map<std::string, std::string> string_map;
	struct Directory {
		std::string base;
		string_map members;
	}; // struct Directory

	bool IsDirectory(std::string const& directory);
	bool Exists(std::string const& file);
	bool Exists(Directory const& dir, std::string const& name);

	std::string MakePath(std::string const& dir, std::string const& name);

	enum Mode { ALL, FILES, DIRECTORIES };
	Directory GetDirectoryMembers(std::string const& dir, Mode m = ALL);

	typedef boost::container::flat_map<std::string, string_map> sub_members_type;
	struct ProjectTree {
		std::string project_path;
		string_map files, directories;
		sub_members_type sub_members;
	}; // struct ProjectTree

	ProjectTree const& GetProjectTree();
	EASYRPG_SHARED_PTR<ProjectTree> CreateProjectTree(std::string const& p);
	bool IsRPG2kProject(ProjectTree const& dir);

} // namespace FileFinder

#endif
