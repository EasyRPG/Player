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

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include "rtp.h"

namespace RTP {
    extern const char* rtp_table_2k[][5];
    extern const char* rtp_table_2k3[][7];

	constexpr int num_2k_rtps = 4;
	constexpr int num_2k3_rtps = 6;

	const char* Names[] {
		"Official Japanese",
		"Official English",
		"Don Miguel English Translation",
		"Don Miguel RTP Addon",
		"Official Japanese",
		"Official English",
		"RPG Advocate English Translation",
		"Vlad Russian Translation",
		"RPG Universe Spanish/Portuguese Translation",
		"Korean Translation"
	};
}

template <typename T>
static void detect_helper(const FileFinder::DirectoryTree& tree, std::vector<struct RTP::RtpHitInfo>& hit_list,
		T rtp_table, int num_rtps, int offset) {
	for (int i = 0; rtp_table[i][0] != nullptr; ++i) {
		const char* category = rtp_table[i][0];
		for (int j = 1; j <= num_rtps; ++j) {
			const char* name = rtp_table[i][j];
			if (name != nullptr) {
				std::string ret;
				// TODO: Filefinder refactor should provide FindImage etc. for non-project trees
				if (!strcmp("sound", category)) {
					// RPG Advocate uses MP3
					static const char* SOUND_TYPES[] = { ".wav", ".mp3", nullptr };
					ret = FileFinder::FindDefault(tree, category, name, SOUND_TYPES);
				} else if (!strcmp("music", category)) {
					static const char* MUSIC_TYPES[] = { ".wav", ".mid", nullptr };
					ret = FileFinder::FindDefault(tree, category, name, MUSIC_TYPES);
				} else if (!strcmp("movie", category)) {
					static const char* MOVIE_TYPES[] = { ".avi", nullptr };
					ret = FileFinder::FindDefault(tree, category, name, MOVIE_TYPES);
				} else {
					static const char* IMAGE_TYPES[] = { ".png", nullptr };
					ret = FileFinder::FindDefault(tree, category, name, IMAGE_TYPES);
				}
				if (!ret.empty()) {
					hit_list[offset + j - 1].hits++;
				}
			}
		}
	}
}

std::vector<RTP::RtpHitInfo> RTP::Detect(std::shared_ptr<FileFinder::DirectoryTree> tree, int version) {
	std::vector<struct RTP::RtpHitInfo> hit_list = {{
		{RTP::Type::RPG2000_OfficialJapanese, Names[0], 2000, 0, 465, tree},
		{RTP::Type::RPG2000_OfficialEnglish, Names[1], 2000, 0, 465, tree},
		{RTP::Type::RPG2000_DonMiguelEnglish, Names[2], 2000, 0, 500, tree},
		{RTP::Type::RPG2000_DonMiguelAddon, Names[3], 2000, 0, 503, tree},
		{RTP::Type::RPG2003_OfficialJapanese, Names[4], 2003, 0, 675, tree},
		{RTP::Type::RPG2003_OfficialEnglish, Names[5], 2003, 0, 675, tree},
		{RTP::Type::RPG2003_RpgAdvocateEnglish, Names[6], 2003, 0, 675, tree},
		{RTP::Type::RPG2003_VladRussian, Names[7], 2003, 0, 350, tree},
		{RTP::Type::RPG2003_RpgUniverseSpanishPortuguese, Names[8], 2003, 0, 600, tree},
		{RTP::Type::RPG2003_Korean, Names[9], 2003, 0, 675, tree}
	}};

	if (version == 2000 || version == 0) {
		detect_helper(*tree, hit_list, rtp_table_2k, num_2k_rtps, 0);
	}
	if (version == 2003 || version == 0) {
		detect_helper(*tree, hit_list, rtp_table_2k3, num_2k3_rtps, num_2k_rtps);
	}

	// remove RTPs with zero hits
	for (auto it = hit_list.begin(); it != hit_list.end(); ) {
		if (it->hits == 0) {
			it = hit_list.erase(it);
		} else {
			++it;
		}
	}

	// sort by hit rate (best to worse)
	std::sort(hit_list.begin(), hit_list.end(), [](const struct RTP::RtpHitInfo& a, const struct RTP::RtpHitInfo& b) {
		return (float)a.hits / a.max > (float)b.hits / b.max;
	});

	return hit_list;
}

template <typename T>
static std::vector<RTP::Type> lookup_any_to_rtp_helper(T rtp_table,
		const std::string &src_category, const std::string &src_name, int num_rtps, int offset) {
	bool cat_found = false;
	std::vector<RTP::Type> type_hits;

	for (int i = 0; rtp_table[i][0] != nullptr; ++i) {
		if (src_category != rtp_table[i][0]) {
			if (cat_found) {
				return type_hits;
			}
			continue;
		}
		cat_found = true;

		for (int j = 1; j <= num_rtps; ++j) {
			const char* name = rtp_table[i][j];
			if (name != nullptr && !strcmp(src_name.c_str(), name)) {
				type_hits.push_back((RTP::Type)(j - 1 + offset));
			}
		}
	}

	return type_hits;
}

std::vector<RTP::Type> RTP::LookupAnyToRtp(const std::string& src_category, const std::string &src_name, int version) {
	if (version == 2000) {
		return lookup_any_to_rtp_helper(rtp_table_2k, src_category, src_name, num_2k_rtps, 0);
	} else {
		return lookup_any_to_rtp_helper(rtp_table_2k3, src_category, src_name, num_2k3_rtps, num_2k_rtps);
	}
}

template <typename T>
static std::string lookup_rtp_to_rtp_helper(T rtp_table,
		const std::string& src_category, const std::string& src_name, int src_index, int dst_index, bool* is_rtp_asset) {
	bool cat_found = false;

	for (int i = 0; rtp_table[i][0] != nullptr; ++i) {
		if (src_category != rtp_table[i][0]) {
			if (cat_found) {
				if (is_rtp_asset) {
					*is_rtp_asset = false;
				}
				return "";
			}
			continue;
		}
		cat_found = true;

		const char* name = rtp_table[i][src_index + 1];
		if (name != nullptr && !strcmp(src_name.c_str(), name)) {
			const char* dst_name = rtp_table[i][dst_index + 1];

			if (is_rtp_asset) {
				*is_rtp_asset = true;
			}

			return dst_name == nullptr ? "" : dst_name;
		}
	}

	if (is_rtp_asset) {
		*is_rtp_asset = false;
	}

	return "";
}

std::string RTP::LookupRtpToRtp(const std::string& src_category, const std::string& src_name, RTP::Type src_rtp,
						   RTP::Type target_rtp, bool* is_rtp_asset) {
	// ensure both 2k or 2k3
	assert((int)src_rtp < num_2k_rtps && (int)target_rtp < num_2k_rtps ||
		(int)src_rtp >= num_2k_rtps && (int)target_rtp >= num_2k_rtps);

	if (src_rtp == target_rtp) {
		// Performance limitation: When game_rtp == installed rtp the code can't tell if it is a rtp asset
		if (is_rtp_asset) {
			*is_rtp_asset = false;
		}
		return src_name;
	}

	if ((int)src_rtp < num_2k_rtps) {
		return lookup_rtp_to_rtp_helper(rtp_table_2k, src_category, src_name, (int)src_rtp, (int)target_rtp, is_rtp_asset);
	} else {
		return lookup_rtp_to_rtp_helper(rtp_table_2k3, src_category, src_name, (int)src_rtp - num_2k_rtps, (int)target_rtp - num_2k_rtps, is_rtp_asset);
	}
}
