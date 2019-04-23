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

#ifndef EP_RTP_H
#define EP_RTP_H

#include <string>
#include <vector>

#include "filefinder.h"

/**
 * The RTP namespace contains functions for working with the Runtime Package
 */
namespace RTP {
	extern const char* const rtp_table_2k[][5];
	extern const char* const rtp_table_2k3[][7];
	extern const char* const rtp_table_2k_categories[15];
	extern const char* const rtp_table_2k3_categories[16];
	extern const int rtp_table_2k_categories_idx[15];
	extern const int rtp_table_2k3_categories_idx[16];

	enum class Type {
		RPG2000_OfficialJapanese,
		RPG2000_OfficialEnglish,
		RPG2000_DonMiguelEnglish,
		RPG2000_DonMiguelAddon,
		RPG2003_OfficialJapanese,
		RPG2003_OfficialEnglish,
		RPG2003_RpgAdvocateEnglish,
		RPG2003_VladRussian,
		RPG2003_RpgUniverseSpanishPortuguese,
		RPG2003_Korean
	};

	extern const char* Names[];

	struct RtpHitInfo {
		RTP::Type type;
		std::string name;
		int version;
		int hits;
		int max;
		std::shared_ptr<FileFinder::DirectoryTree> tree;
	};

	/**
	 * Uses a file finder tree to detect the installed RTP, the list is sorted from best to worst hit.
	 * RTP with zero hits are removed.
	 *
	 * @param tree directory tree
	 * @param version RTP version in the folder (2000 or 2003), use 0 to detect all
	 * @return List of detected RTP types
	 */
	std::vector<RtpHitInfo> Detect(std::shared_ptr<FileFinder::DirectoryTree> tree, int version);

	/**
	 * Takes an asset name and returns all RTP that match
	 *
	 * @param src_category Asset category
	 * @param src_name Asset name
	 * @param version rtp version (2000 or 2003)
	 * @return List of possible RTP
	 */
	std::vector<RTP::Type> LookupAnyToRtp(const std::string& src_category, const std::string& src_name, int version);

	/**
	 * Takes a source and a destination RTP. Maps source name to destination name and returns it.
	 *
	 * @param src_category Asset category
	 * @param src_name Asset name
	 * @param src_rtp Type of the source RTP the asset name is from
	 * @param target_rtp Target RTP the asset name is translated to
	 * @param is_rtp_asset When not null indicates if src_name is a known entry in the src_rtp but only when src_rtp != target_rtp
	 * @return The translated asset name or empty string when the mapping is unavailable
	 */
	std::string LookupRtpToRtp(const std::string& src_category, const std::string& src_name,
		RTP::Type src_rtp, RTP::Type target_rtp, bool* is_rtp_asset = nullptr);
}

#endif