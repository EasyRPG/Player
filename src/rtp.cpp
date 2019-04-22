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
					static const char* SOUND_TYPES[] = { ".wav", nullptr };
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
					hit_list[offset + j].hits++;
				}
			}
		}
	}
}

std::vector<RTP::RtpHitInfo> RTP::Detect(std::shared_ptr<FileFinder::DirectoryTree> tree, int version) {
	std::vector<struct RTP::RtpHitInfo> hit_list = {{
		{RTP::Type::RPG2000_OfficialJapanese, "Official Japanese", 2000, 0, 465, tree},
		{RTP::Type::RPG2000_OfficialEnglish, "Official English", 2000, 0, 465, tree},
		{RTP::Type::RPG2000_DonMiguelEnglish, "Don Miguel English Translation", 2000, 0, 500, tree},
		{RTP::Type::RPG2000_DonMigualAddon, "Don Miguel RTP Addon", 2000, 0, 503, tree},
		{RTP::Type::RPG2003_OfficialJapanese, "Official Japanese", 2003, 0, 675, tree},
		{RTP::Type::RPG2003_OfficialEnglish, "Official English", 2003, 0, 675, tree},
		{RTP::Type::RPG2003_RpgAdvocateEnglish, "RPG Advocate English Translation", 2003, 0, 675, tree},
		{RTP::Type::RPG2003_VladRussian, "Vlad Russian Translation", 2003, 0, 350, tree},
		{RTP::Type::RPG2003_RpgUniverseSpanishPortuguese, "RPG Universe Spanish/Portuguese Translation", 2003, 0, 600, tree},
		{RTP::Type::RPG2003_Korean, "Korean Translation", 2003, 0, 675, tree}
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

	// sort by best hit rate
	std::sort(hit_list.begin(), hit_list.end(), [](const struct RTP::RtpHitInfo& a, const struct RTP::RtpHitInfo& b) {
		return (float)a.hits / a.max < (float)b.hits / a.max;
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
		const std::string& src_category, const std::string& src_name, int src_index, int dst_index) {
	bool cat_found = false;

	for (int i = 0; rtp_table[i][0] != nullptr; ++i) {
		if (src_category != rtp_table[i][0]) {
			if (cat_found) {
				return "";
			}
			continue;
		}
		cat_found = true;

		const char* name = rtp_table[i][src_index];
		if (name != nullptr && !strcmp(src_name.c_str(), name)) {
			return rtp_table[i][dst_index];
		}
	}

	return "";
}

std::string RTP::LookupRtpToRtp(const std::string& src_category, const std::string& src_name, RTP::Type src_rtp,
						   RTP::Type target_rtp) {
	// ensure both 2k or 2k3
	assert((int)src_rtp < num_2k_rtps && (int)target_rtp < num_2k_rtps ||
		(int)src_rtp >= num_2k_rtps && (int)target_rtp >= num_2k_rtps);

	if ((int)src_rtp < num_2k_rtps) {
		return lookup_rtp_to_rtp_helper(rtp_table_2k, src_category, src_name, (int)src_rtp, (int)target_rtp);
	} else {
		return lookup_rtp_to_rtp_helper(rtp_table_2k3, src_category, src_name, (int)src_rtp - num_2k_rtps, (int)target_rtp - num_2k_rtps);
	}
}
