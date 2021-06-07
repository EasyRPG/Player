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

// Headers

#include "filefinder.h"
#include "fileext_guesser.h"
#include "meta.h"
#include "output.h"
#include "string_view.h"


namespace {
	const std::string RtPrefix = "rpg_rt.";
	const std::string MapPrefix = "map";
	const size_t MapPrefixLength = 8; // mapXXXX.
}


FileExtGuesser::RPG2KNonStandardFilenameGuesser FileExtGuesser::GetRPG2kProjectWithRenames(const FilesystemView& fs) {
	// Try to rescue and determine file extensions.
	// We need to figure out the names of the map tree and the DB (maps come later).
	std::vector<RPG2KNonStandardFilenameGuesser::ExtAndSize> candidates;
	const auto* entries = fs.ListDirectory();
	if (entries) {
		for (const auto &item: *entries) {
			if (item.second.type != DirectoryTree::FileType::Regular) {
				continue;
			}

			if (item.first.length() == RtPrefix.length() + 3 && ToStringView(item.first).starts_with(RtPrefix)) {
				std::string ext = item.first.substr(RtPrefix.length());
				if (ext != "exe" && ext != "ini") {
					candidates.emplace_back(item.second.name, ext, fs.GetFilesize(fs.FindFile(item.second.name)));
				}
			}

			// Avoid needless scanning if we can't figure it out
			if (candidates.size() > 2) {
				break;
			}
		}

		// Return only if we matched exactly two files.
		if (candidates.size() == 2) {
			RPG2KNonStandardFilenameGuesser res;
			res.rpgRTs.first = candidates[0];
			res.rpgRTs.second = candidates[1];
			return res;
		}
	}

	return RPG2KNonStandardFilenameGuesser();
}

void FileExtGuesser::GuessAndAddLmuExtension(const FilesystemView& fs, Meta const& meta, RPG2KFileExtRemap& mapping)
{
	// If metadata is provided, rely on that.
	std::string metaLmu = meta.GetLmuAlias();
	if (!metaLmu.empty()) {
		mapping.extMap[SUFFIX_LMU] = metaLmu;
		Output::Debug("Metadata-provided non-standard extension for LMU({})", metaLmu);
	} else {
		// Try to rescue and determine file extensions.
		// Without metadata, scan for matching files. Stop after you find a few;
		//   we can't just pick the first since there may be some backup files on disk.
		std::unordered_map<std::string, int> extCounts; // ext => count

		const auto* entries = fs.ListDirectory();
		if (entries) {
			for (const auto &item: *entries) {
				if (item.second.type != DirectoryTree::FileType::Regular) {
					continue;
				}

				if (item.first.length() == MapPrefixLength + 3 &&
					StringView(item.first).starts_with(MapPrefix)) {
					std::string ext = item.first.substr(MapPrefixLength);
					extCounts[ext] += 1;
					if (extCounts[ext] >= 5) {
						mapping.extMap[SUFFIX_LMU] = ext;
						Output::Debug("Guessing non-standard extension for LMU({})", ext);
						break;
					}
				}
			}
		}
	}
}

FileExtGuesser::RPG2KFileExtRemap FileExtGuesser::RPG2KNonStandardFilenameGuesser::guessExtensions(Meta& meta)
{
	RPG2KFileExtRemap res;

	// Since the file extensions are non-standard, we
	// won't have CRCs for them, so we need to guess more
	if (!this->Empty()) {
		meta.ReInitForNonStandardExtensions(rpgRTs.first.fname, rpgRTs.second.fname);
	}

	// If metadata exists, we don't need to guess
	std::string metaLdb = meta.GetLdbAlias();
	std::string metaLmt = meta.GetLmtAlias();
	if (!metaLdb.empty()) {
		res.extMap[SUFFIX_LDB] = metaLdb;
	}
	if (!metaLmt.empty()) {
		res.extMap[SUFFIX_LMT] = metaLmt;
	}

	// If neither exists, we have to fall back to guessing.
	if (!(metaLdb.empty() && metaLmt.empty())) {
		Output::Debug("Metadata-provided non-standard extension for LDB({}) and LMT({})", metaLdb, metaLmt);
	} else {
		// With no metadata, just assume the largest file is the database
		// It's usually bigger by a factor of 10.
		auto first = rpgRTs.first;
		auto second = rpgRTs.second;
		if (first.sz > second.sz) {
			res.extMap[SUFFIX_LDB] = first.ext;
			res.extMap[SUFFIX_LMT] = second.ext;
		} else {
			res.extMap[SUFFIX_LDB] = second.ext;
			res.extMap[SUFFIX_LMT] = first.ext;
		}

		Output::Debug("Guessing non-standard extensions for LDB({}) and LMT({})", res.extMap[SUFFIX_LDB], res.extMap[SUFFIX_LMT]);
	}

	return res;
}

bool FileExtGuesser::RPG2KNonStandardFilenameGuesser::Empty() const {
	return rpgRTs.first.ext.empty() || rpgRTs.second.ext.empty();
}

std::string FileExtGuesser::RPG2KFileExtRemap::MakeFilename(StringView prefix, StringView suffix)
{
	std::stringstream res;
	res <<prefix <<".";

	auto it = extMap.find(ToString(suffix));
	if (it != extMap.end()) {
		res << it->second;
	} else {
		res << suffix;
	}
	return res.str();
}
