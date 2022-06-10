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
		"Korean Translation",
		"Official Traditional Chinese"
	};
}

static std::pair<int, int> get_table_idx(const char* const lookup_table[16], const int lookup_table_idx[16], StringView category) {
	int i;

	for (i = 0; lookup_table[i] != nullptr; ++i) {
		if (StringView(lookup_table[i]) == category) {
			return {lookup_table_idx[i], lookup_table_idx[i+1]};
		}
	}

	// Points at nullptr (final row) in the rtp table
	return {lookup_table_idx[i], lookup_table_idx[i]};
}

template <typename T>
static void detect_helper(const FilesystemView& fs, std::vector<struct RTP::RtpHitInfo>& hit_list,
		T rtp_table, int num_rtps, int offset, const std::pair<int, int>& range, Span<StringView> ext_list, int miss_limit) {
	std::string ret;
	for (int j = 1; j <= num_rtps; ++j) {
		int cur_miss = 0;
		for (int i = range.first; i < range.second; ++i) {
			const char* category = rtp_table[i][0];
			const char* name = rtp_table[i][j];
			if (name != nullptr) {
				// TODO: Filefinder refactor should provide FindImage etc. for non-project trees
				DirectoryTree::Args args = { FileFinder::MakePath(category, name), ext_list, 1, false };
				ret = fs.FindFile(args);
				if (!ret.empty()) {
					hit_list[offset + j - 1].hits++;
				} else {
					++cur_miss;
					if (cur_miss > miss_limit) {
						break;
					}
				}
			}
		}
	}
}

std::vector<RTP::RtpHitInfo> RTP::Detect(const FilesystemView& fs, int version, int miss_limit) {
	std::vector<struct RTP::RtpHitInfo> hit_list = {{
		{RTP::Type::RPG2000_OfficialJapanese, Names[0], 2000, 0, 465, fs},
		{RTP::Type::RPG2000_OfficialEnglish, Names[1], 2000, 0, 465, fs},
		{RTP::Type::RPG2000_DonMiguelEnglish, Names[2], 2000, 0, 500, fs},
		{RTP::Type::RPG2000_DonMiguelAddon, Names[3], 2000, 0, 503, fs},
		{RTP::Type::RPG2003_OfficialJapanese, Names[4], 2003, 0, 675, fs},
		{RTP::Type::RPG2003_OfficialEnglish, Names[5], 2003, 0, 675, fs},
		{RTP::Type::RPG2003_RpgAdvocateEnglish, Names[6], 2003, 0, 675, fs},
		{RTP::Type::RPG2003_VladRussian, Names[7], 2003, 0, 350, fs},
		{RTP::Type::RPG2003_RpgUniverseSpanishPortuguese, Names[8], 2003, 0, 600, fs},
		{RTP::Type::RPG2003_Korean, Names[9], 2003, 0, 675, fs},
		{RTP::Type::RPG2003_OfficialTraditionalChinese, Names[10], 2003, 0, 676, fs}
	}};

	auto SOUND_TYPES = Utils::MakeSvVector(".wav", ".mp3");
	auto MUSIC_TYPES = Utils::MakeSvVector(".wav", ".mid");
	auto MOVIE_TYPES = Utils::MakeSvVector(".avi");
	auto IMAGE_TYPES = Utils::MakeSvVector(".png");

	auto ext_for_cat = [=](const char* category) {
		if (!strcmp("sound", category)) {
			return SOUND_TYPES;
		} else if (!strcmp("music", category)) {
			return MUSIC_TYPES;
		} else if (!strcmp("movie", category)) {
			return MOVIE_TYPES;
		} else {
			return IMAGE_TYPES;
		}
	};

	if (version == 2000 || version == 0) {
		for (int i = 0; rtp_table_2k_categories[i] != nullptr; ++i) {
			const char* category = rtp_table_2k_categories[i];
			std::pair<int, int> range = {rtp_table_2k_categories_idx[i], rtp_table_2k_categories_idx[i+1]};
			auto ext_list = ext_for_cat(category);
			detect_helper(fs, hit_list, rtp_table_2k, num_2k_rtps, 0, range, ext_list, miss_limit);
		}
	}
	if (version == 2003 || version == 0) {
		for (int i = 0; rtp_table_2k3_categories[i] != nullptr; ++i) {
			const char* category = rtp_table_2k3_categories[i];
			std::pair<int, int> range = {rtp_table_2k3_categories_idx[i], rtp_table_2k3_categories_idx[i+1]};
			auto ext_list = ext_for_cat(category);
			detect_helper(fs, hit_list, rtp_table_2k3, num_2k3_rtps, num_2k_rtps, range, ext_list, miss_limit);
		}
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
static std::vector<RTP::Type> lookup_any_to_rtp_helper(T rtp_table, const std::pair<int, int>& range,
		StringView src_name, int num_rtps, int offset) {
	std::vector<RTP::Type> type_hits;

	for (int i = range.first; i < range.second; ++i) {
		for (int j = 1; j <= num_rtps; ++j) {
			const char* name = rtp_table[i][j];
			if (name != nullptr && src_name == StringView(name)) {
				type_hits.push_back((RTP::Type)(j - 1 + offset));
			}
		}
	}

	return type_hits;
}

std::vector<RTP::Type> RTP::LookupAnyToRtp(StringView src_category, StringView src_name, int version) {
	if (version == 2000) {
		auto tbl_idx = get_table_idx(rtp_table_2k_categories, rtp_table_2k_categories_idx, src_category);
		return lookup_any_to_rtp_helper(rtp_table_2k, tbl_idx, src_name, num_2k_rtps, 0);
	} else {
		auto tbl_idx = get_table_idx(rtp_table_2k3_categories, rtp_table_2k3_categories_idx, src_category);
		return lookup_any_to_rtp_helper(rtp_table_2k3, tbl_idx, src_name, num_2k3_rtps, num_2k_rtps);
	}
}

template <typename T>
static std::string lookup_rtp_to_rtp_helper(T rtp_table, const std::pair<int, int>& range,
		StringView src_name, int src_index, int dst_index, bool* is_rtp_asset) {

	for (int i = range.first; i < range.second; ++i) {
		const char* name = rtp_table[i][src_index + 1];
		if (name != nullptr && src_name == StringView(name)) {
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

std::string RTP::LookupRtpToRtp(StringView src_category, StringView src_name, RTP::Type src_rtp,
		RTP::Type target_rtp, bool* is_rtp_asset) {
	// ensure both 2k or 2k3
	assert(((int)src_rtp < num_2k_rtps && (int)target_rtp < num_2k_rtps) ||
		((int)src_rtp >= num_2k_rtps && (int)target_rtp >= num_2k_rtps));

	if (src_rtp == target_rtp) {
		// Design limitation: When game_rtp == installed rtp can't tell if it is a rtp asset, this needs a table scan
		if (is_rtp_asset) {
			*is_rtp_asset = false;
		}
		return ToString(src_name);
	}

	if ((int)src_rtp < num_2k_rtps) {
		auto tbl_idx = get_table_idx(rtp_table_2k_categories, rtp_table_2k_categories_idx, src_category);
		return lookup_rtp_to_rtp_helper(rtp_table_2k, tbl_idx, src_name, (int)src_rtp, (int)target_rtp, is_rtp_asset);
	} else {
		auto tbl_idx = get_table_idx(rtp_table_2k3_categories, rtp_table_2k3_categories_idx, src_category);
		return lookup_rtp_to_rtp_helper(rtp_table_2k3, tbl_idx, src_name, (int)src_rtp - num_2k_rtps, (int)target_rtp - num_2k_rtps, is_rtp_asset);
	}
}
