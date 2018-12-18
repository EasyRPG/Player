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

#ifndef EP_RTP_TABLE_H
#define EP_RTP_TABLE_H

#include <map>
#include <string>
#include <cstring>

namespace RTP {
	struct rtp_table_cmp {
		bool operator()(char const* a, char const* b) const {
			return std::strcmp(a, b) < 0;
		}
	};

	using rtp_table_type = std::map<const char*, std::map<const char*, const char*, rtp_table_cmp>, rtp_table_cmp>;

	void Init();

	/**
	 * { folder: { ASCII, UTF-8 } }
	 */
	extern rtp_table_type RTP_TABLE_2003;
	extern rtp_table_type RTP_TABLE_2000;
}

#endif
