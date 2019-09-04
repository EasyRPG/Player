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

#include <iostream>

// Headers
#include <fstream>
#include <iomanip>
#include <zlib.h>
#include "data.h"
#include "filefinder.h"
#include "lmt_reader.h"
#include "lsd_reader.h"
#include "main_data.h"
#include "meta.h"
#include "output.h"
#include "player.h"
#include "reader_util.h"


// Constants used for identifying fields in the easyrpg.ini file.
#define MTINI_NAME "Name"
#define MTINI_IMPORT_SAVE_PIVOT_MAP "ImportSavePivotMap"
#define MTINI_IMPORT_SAVE_PARENT "ImportSaveParent"
#define MTINI_EASY_RPG_SECTION "EasyRPG"
#define MTINI_FILE_FORMAT_VERSION "FileFormatVersion"

// Extended vocab key/value pairs
#define MTINI_EXVOCAB_IMPORT_SAVE_HELP_KEY "Vocab_ImportSaveHelp"
#define MTINI_EXVOCAB_IMPORT_SAVE_HELP_VALUE "Import Existing Save (Multi-Games Only)"
#define MTINI_EXVOCAB_IMPORT_SAVE_TITLE_KEY "Vocab_ImportSave"
#define MTINI_EXVOCAB_IMPORT_SAVE_TITLE_VALUE "Import Save"


// Helper: Get the CRC32 of a given file as a hex string
std::string crc32file(std::string file_name) {
	if (!file_name.empty()) {
		std::ifstream in(file_name.c_str(), std::ios::binary);
		if (in.is_open()) {
			std::string buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			unsigned long crc = ::crc32(0, reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.length());

			std::stringstream res;
			res <<std::hex <<std::setfill('0') <<std::setw(8) <<crc;
			return res.str();
		}
	}
	return "";
}


Meta::Meta(const std::string& meta_file) {
	ini.reset(new INIReader(meta_file));

	// Cache per-game lookups
	if (!Empty()) {
		std::string version = ini->GetString(MTINI_EASY_RPG_SECTION, MTINI_FILE_FORMAT_VERSION, "");
		if (version == "1") {
			IdentifyCanonName();
		} else {
			Output::Warning("Metadata error in %s, format property %s:%s is missing or invalid: '%s'", meta_file.c_str(), MTINI_EASY_RPG_SECTION, MTINI_FILE_FORMAT_VERSION, version.c_str());
		}
	}

}

int Meta::GetPivotMap() const {
	if (!Empty()) {
		return ini->GetInteger(canon_ini_lookup, MTINI_IMPORT_SAVE_PIVOT_MAP, 0);
	}

	return 0;
}

std::string Meta::GetParentGame() const {
	if (!Empty()) {
		return ini->GetString(canon_ini_lookup, MTINI_IMPORT_SAVE_PARENT, "");
	}

	return "";
}

std::vector<std::string> Meta::GetImportChildPaths(const FileFinder::DirectoryTree& parent_tree) const {
	std::vector<std::string> res;
	if (!Empty()) {
		for (const auto& paths : parent_tree.directories) {
			res.push_back(paths.second);
		}
	}
	return res;
}

std::vector<Meta::FileItem> Meta::SearchImportPaths(const FileFinder::DirectoryTree& parent_tree, const std::string& child_path) const {
	if (!Empty()) {
		int pivotMapId = GetPivotMap();
		auto parent = GetParentGame();
		return BuildImportCandidateList(parent_tree, child_path, parent, pivotMapId);
	}

	return std::vector<Meta::FileItem>();
}


std::vector<Meta::FileItem> Meta::BuildImportCandidateList(const FileFinder::DirectoryTree& parent_tree, const std::string& child_path, const std::string& parent_game_name, int pivot_map_id) const {
	// Scan each folder, looking for an ini file
	// For now, this only works with "standard" folder layouts, since we need Game files + Save files
	std::vector<Meta::FileItem> res;

	// Try to read the game name. Note that we assume the games all have the same encoding (and use Player::encoding)
	auto child_full_path = FileFinder::MakePath(parent_tree.directory_path, child_path);
	auto child_tree = FileFinder::CreateDirectoryTree(child_full_path, FileFinder::FILES);
	bool is_match = false;
	if (child_tree != nullptr) { 
		// Try to match the parent game name
		std::string lmtPath = FileFinder::FindDefault(*child_tree, TREEMAP_NAME);
		std::string crcLMT = crc32file(lmtPath);
		std::string crcLDB = "*";
		if (parent_game_name.find(crcLMT)==0) {
			if (parent_game_name == crcLMT + "/" + crcLDB) {
				is_match = true;
			} else {
				std::string ldbPath = FileFinder::FindDefault(*child_tree, DATABASE_NAME);
				crcLDB = crc32file(ldbPath);
				if (parent_game_name == crcLMT + "/" + crcLDB) {
					is_match = true;
				}
			}
		}
	}

	if (is_match) {
		// Scan over every possible save file and see if any match.
		for (int saveId = 0; saveId < 15; saveId++) {
			std::stringstream ss;
			ss << "Save" << (saveId <= 8 ? "0" : "") << (saveId + 1) << ".lsd";

			// Check for an existing, non-corrupt file with the right mapID
			// Note that corruptness is checked later (in window_savefile.cpp)
			std::string file = FileFinder::FindDefault(*child_tree, ss.str());
			if (!file.empty()) {
				std::unique_ptr<RPG::Save> savegame = LSD_Reader::Load(file, Player::encoding);
				if (savegame != nullptr) {
					if (savegame->party_location.map_id == pivot_map_id || pivot_map_id==0) {
						FileItem item;
						item.full_path = file;
						item.short_path = FileFinder::MakeCanonical(child_path, 1);
						item.file_id = saveId + 1;
						res.push_back(item);
					}
				}
			}
		}
	}

	return res;
}

bool Meta::IsImportEnabled() const {
	return !GetParentGame().empty();
}


std::string Meta::GetExVocabImportSaveHelpText() const {
	return GetExVocab(MTINI_EXVOCAB_IMPORT_SAVE_HELP_KEY, MTINI_EXVOCAB_IMPORT_SAVE_HELP_VALUE);
}

std::string Meta::GetExVocabImportSaveTitleText() const {
	return GetExVocab(MTINI_EXVOCAB_IMPORT_SAVE_TITLE_KEY, MTINI_EXVOCAB_IMPORT_SAVE_TITLE_VALUE);
}

std::string Meta::GetExVocab(const std::string& term, const std::string& def_value) const {
	if (!Empty()) {
		return ini->GetString(canon_ini_lookup, term, def_value);
	}

	return def_value;
}

void Meta::IdentifyCanonName() {
	std::string lmtPath = FileFinder::FindDefault(TREEMAP_NAME);

	// Calculate the lookup based on the LMT/LDB hashes, preferring to use LMT only if possible.
	// This requires a mandatory field, for which we will use "Name".
	if (!Empty()) {
		std::string crcLMT = crc32file(lmtPath);
		std::string crcLDB = "*";
		if (ini->HasValue(crcLMT + "/" + crcLDB , MTINI_NAME)) {
			canon_ini_lookup = crcLMT + "/" + crcLDB;
		} else {
			std::string ldbPath = FileFinder::FindDefault(DATABASE_NAME);
			crcLDB = crc32file(ldbPath);
			if (ini->HasValue(crcLMT + "/" + crcLDB , MTINI_NAME)) {
				canon_ini_lookup = crcLMT + "/" + crcLDB;
			}
		}
	}
}

bool Meta::Empty() const {
	return ini == nullptr || ini->ParseError() == -1;
}
