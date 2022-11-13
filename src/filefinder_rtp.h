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

#ifndef EP_FILEFINDER_RTP_H
#define EP_FILEFINDER_RTP_H

#include "directory_tree.h"
#include "rtp.h"
#include "string_view.h"

class FileFinder_RTP {
public:
	/**
	 * Manages RTP folders.
	 *
	 * @param no_rtp If true disables RTP support completely
	 * @param no_rtp_warnings If true disables warnings when a RTP asset is used
	 * @param rtp_path Custom RTP path to use
	 */
	FileFinder_RTP(bool no_rtp, bool no_rtp_warnings, std::string rtp_path);

	/**
	 * Looks up a file in the list of RTPs
	 *
	 * @param dir Directory containing the file
	 * @param name Filename
	 * @param exts Extensions to probe
	 * @return A handle to the file or an invalid handle if not found
	 */
	 Filesystem_Stream::InputStream Lookup(StringView dir, StringView name, const Span<const StringView> exts) const;

private:
	void AddPath(StringView p);
	void ReadRegistry(StringView company, StringView product, StringView key);
	Filesystem_Stream::InputStream LookupInternal(StringView dir, StringView name, const Span<const StringView> exts, bool& is_rtp_asset) const;

	using search_path_list = std::vector<FilesystemView>;

	/** all RTP search paths */
	search_path_list search_paths;
	/** RTP was disabled with --disable-rtp */
	bool disable_rtp = true;
	/** Game has FullPackageFlag=1, RTP will still be used as RPG_RT does */
	bool game_has_full_package_flag = false;
	/** warning about "game has FullPackageFlag=1 but needs RTP" shown */
	mutable bool warning_broken_rtp_game_shown = false;
	/** RTP candidates per search_path */
	std::vector<RTP::RtpHitInfo> detected_rtp;
	/** the RTP the game uses, when only one left the RTP of the game is known */
	mutable std::vector<RTP::Type> game_rtp;
};

#endif
