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

#ifndef EP_FEATURE_H
#define EP_FEATURE_H

/**
 * Feature namespace.
 */
namespace Feature {
	/**
	 * @return true if the RPG Maker 2000 battle system is used
	 */
	bool HasRpg2kBattleSystem();

	/**
	 * @return true if the RPG Maker 2003 battle system is used
	 */
	bool HasRpg2k3BattleSystem();

	/**
	 * @return true if the row feature is enabled
	 */
	bool HasRow();

	/**
	 * @return true if text placeholders are used
	 */
	bool HasPlaceholders();
}

#endif
