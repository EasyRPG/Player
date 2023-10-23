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

#ifndef EP_DESTINY_H
#define EP_DESTINY_H

#include <cstdint>


namespace Destiny {
	constexpr const char* DESTINY_DLL = "Destiny.dll";

	void Load();
	void Initialize(
		uint32_t _dllVersion,
		uint32_t _language,
		uint32_t _gameVersion,
		uint32_t _extra,
		uint32_t _dwordSize,
		uint32_t _floatSize,
		uint32_t _stringSize
	);
}

#endif // !EP_DESTINY_H
