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

#include "drawable.h"

int Drawable::GetPriorityForMapLayer(int which) const {
	switch (which) {
		case 1:
			return Priority_Background;
		case 2:
			return Priority_TilesetBelow;
		case 3:
			return Priority_EventsBelow;
		case 4:
			return Priority_Player;
		case 5:
			return Priority_TilesetAbove;
		case 6:
			return Priority_EventsAbove;
		case 7:
			return Priority_PictureNew;
		case 8:
			return Priority_BattleAnimation;
		case 9:
			return Priority_Window;
		case 10:
			return Priority_Timer;
		default:
			return 0;
	}
}

int Drawable::GetPriorityForBattleLayer(int which) const {
	switch (which) {
		case 1:
			return Priority_Background;
		case 2:
			return Priority_Battler;
		case 3:
			return Priority_PictureNew;
		case 4:
			return Priority_Window;
		case 5:
			return Priority_Timer;
		default:
			return 0;
	}
}

#endif
