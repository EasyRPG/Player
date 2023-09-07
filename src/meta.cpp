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
#include <memory>
#include <sstream>
#include <lcf/data.h>
#include "filefinder.h"
#include <lcf/lmt/reader.h>
#include <lcf/lsd/reader.h>
#include "main_data.h"
#include "meta.h"
#include "output.h"
#include "player.h"
#include "translation.h"
#include <lcf/reader_util.h>


// Constants used for identifying fields in the easyrpg.ini file.
#define MTINI_NAME "Name"
#define MTINI_IMPORT_SAVE_PIVOT_MAP "ImportSavePivotMap"
#define MTINI_IMPORT_SAVE_PARENT "ImportSaveParent"
#define MTINI_EASY_RPG_SECTION "EasyRPG"
#define MTINI_FILE_FORMAT_VERSION "FileFormatVersion"
#define MTINI_FILEXT_LDB_ALIAS "LdbFileAlias"
#define MTINI_FILEXT_LMT_ALIAS "LmtFileAlias"
#define MTINI_FILEXT_LMU_ALIAS "LmuFileAlias"

// Extended vocab key/value pairs
#define MTINI_EXVOCAB_IMPORT_SAVE_HELP_KEY "Vocab_ImportSaveHelp"
#define MTINI_EXVOCAB_IMPORT_SAVE_HELP_VALUE "Import Existing Save (Multi-Games Only)"
#define MTINI_EXVOCAB_IMPORT_SAVE_TITLE_KEY "Vocab_ImportSave"
#define MTINI_EXVOCAB_IMPORT_SAVE_TITLE_VALUE "Import Save"
#define MTINI_EXVOCAB_TRANSLATE_TITLE_KEY "Vocab_Translate"
#define MTINI_EXVOCAB_TRANSLATE_TITLE_VALUE "Language"


// Helper: Get the CRC32 of a given file as a hex string
std::string crc32file(std::string file_name) {
	if (!file_name.empty()) {
		auto in = FileFinder::Game().OpenInputStream(file_name);
		if (in) {
			auto crc = Utils::CRC32(in);
			std::stringstream res;
			res <<std::hex << std::setfill('0') <<std::setw(8) <<crc;
			return res.str();
		}
	}
	return "";
}


Meta::Meta(StringView meta_file) {
	ini = std::make_unique<lcf::INIReader>(ToString(meta_file));

	// Cache per-game lookups
	if (!Empty()) {
		std::string version = ini->GetString(MTINI_EASY_RPG_SECTION, MTINI_FILE_FORMAT_VERSION, "");
		if (version == "1") {
			IdentifyCanonName(TREEMAP_NAME, DATABASE_NAME);
		} else {
			Output::Warning("Metadata error in {}, format property {}:{} is missing or invalid: '{}'", meta_file, MTINI_EASY_RPG_SECTION, MTINI_FILE_FORMAT_VERSION, version);
			ini = nullptr;
		}
	}

}

void Meta::ReInitForNonStandardExtensions(StringView file1, StringView file2) {
	if (!Empty()) {
		if (canon_ini_lookup.empty()) {
			IdentifyCanonName(file1, file2);
		}
		if (canon_ini_lookup.empty()) {
			IdentifyCanonName(file2, file1);
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

std::vector<std::string> Meta::GetImportChildPaths(const FilesystemView& parent_fs) const {
	std::vector<std::string> res;

	if (!Empty()) {
		const auto* entries = parent_fs.ListDirectory();
		if (entries) {
			for (const auto &item: *entries) {
				if (item.second.type != DirectoryTree::FileType::Directory) {
					continue;
				}
				res.push_back(item.second.name);
			}
		}
	}

	return res;
}

std::vector<Meta::FileItem> Meta::SearchImportPaths(const FilesystemView& parent_fs, StringView child_path) const {
	if (!Empty()) {
		int pivotMapId = GetPivotMap();
		auto parent = GetParentGame();
		return BuildImportCandidateList(parent_fs, child_path, parent, pivotMapId);
	}

	return std::vector<Meta::FileItem>();
}


std::vector<Meta::FileItem> Meta::BuildImportCandidateList(const FilesystemView& parent_fs, StringView child_path, StringView parent_game_name, int pivot_map_id) const {
	// Scan each folder, looking for an ini file
	// For now, this only works with "standard" folder layouts, since we need Game files + Save files
	std::vector<Meta::FileItem> res;

	// Try to read the game name. Note that we assume the games all have the same encoding (and use Player::encoding)
	bool is_match = false;

	// NOTE: FindFile doesn't work outside the root of the FileSystemView, so we use Exists() and pass a relative path.
	//       This will likely only work on Native filesystems; the Exists(".") attempts to sanity check this.
	auto child_tree = parent_fs.Subtree(child_path);
	if (child_tree.Exists(".")) {
		// Try to match the parent game name.
		std::string lmtPath = child_tree.GetSubPath() + "/" + TREEMAP_NAME;
		std::string crcLMT = crc32file(lmtPath);
		std::string crcLDB = "*";

		if (parent_game_name.find(crcLMT)==0) {
			if (parent_game_name == crcLMT + "/" + crcLDB) {
				is_match = true;
			} else {
				std::string ldbPath = child_tree.GetSubPath() + "/" + DATABASE_NAME;
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
			if (child_tree.Exists(ss.str())) {
				auto filePath= child_tree.GetSubPath() + "/" + ss.str();
				std::unique_ptr<lcf::rpg::Save> savegame = lcf::LSD_Reader::Load(filePath, Player::encoding);
				if (savegame != nullptr) {
					if (savegame->party_location.map_id == pivot_map_id || pivot_map_id==0) {
						FileItem item;
						item.full_path = filePath;
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

std::string Meta::GetLdbAlias() const {
	if (!Empty()) {
		return ini->GetString(canon_ini_lookup, MTINI_FILEXT_LDB_ALIAS, "");
	}
	return "";
}

std::string Meta::GetLmtAlias() const {
	if (!Empty()) {
		return ini->GetString(canon_ini_lookup, MTINI_FILEXT_LMT_ALIAS, "");
	}
	return "";
}

std::string Meta::GetLmuAlias() const {
	if (!Empty()) {
		return ini->GetString(canon_ini_lookup, MTINI_FILEXT_LMU_ALIAS, "");
	}
	return "";
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

std::string Meta::GetExVocabTranslateTitleText() const {
	std::string term;

	if (Tr::HasActiveTranslation()) {
		term = Player::translation.GetCurrentLanguage().lang_term;
	} else {
		term = Player::translation.GetDefaultLanguage().lang_term;
	}

	if (!term.empty()) {
		return term;
	}

	return GetExVocab(MTINI_EXVOCAB_TRANSLATE_TITLE_KEY, MTINI_EXVOCAB_TRANSLATE_TITLE_VALUE);
}

std::string Meta::GetExVocab(StringView term, StringView def_value) const {
	if (!Empty()) {
		return ini->GetString(canon_ini_lookup, ToString(term), ToString(def_value));
	}

	return ToString(def_value);
}

void Meta::IdentifyCanonName(StringView lmtFile, StringView ldbFile) {
	// Calculate the lookup based on the LMT/LDB hashes, preferring to use LMT only if possible.
	// This requires a mandatory field, for which we will use "Name".
	if (!Empty()) {
		std::string lmtPath = FileFinder::Game().FindFile(ToString(lmtFile));
		std::string crcLMT = crc32file(lmtPath);
		std::string crcLDB = "*";
		Output::Debug("CRC32 of 'LMT' file ('{}') is {}", lmtFile, crcLMT);
		if (ini->HasValue(crcLMT + "/" + crcLDB , MTINI_NAME)) {
			canon_ini_lookup = crcLMT + "/" + crcLDB;
		} else {
			std::string ldbPath = FileFinder::Game().FindFile(ToString(ldbFile));
			crcLDB = crc32file(ldbPath);
			if (ini->HasValue(crcLMT + "/" + crcLDB , MTINI_NAME)) {
				canon_ini_lookup = crcLMT + "/" + crcLDB;
			}
			Output::Debug("CRC32 of 'LDB' file ('{}') file is {}", ldbFile, crcLDB);
		}
	}
}

bool Meta::Empty() const {
	return ini == nullptr || ini->ParseError() == -1;
}
