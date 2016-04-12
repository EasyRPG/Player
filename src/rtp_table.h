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

#ifndef _RTP_TABLE_H_
#define _RTP_TABLE_H_

#include <map>
#include <string>

typedef std::map<std::string, std::map<std::string, std::string>> rtp_table_type;

namespace RTP {
	void Init();

	/**
	 * { folder: { ASCII, UTF-8 } }
	 */
	extern rtp_table_type RTP_TABLE_2003;
	extern rtp_table_type RTP_TABLE_2000;
}

#endif
