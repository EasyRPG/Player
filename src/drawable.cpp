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
#include "rpg_savepicture.h"

int Drawable::GetPriorityForMapLayer(int which) {
	switch (which) {
		case RPG::SavePicture::MapLayer_parallax:
			return Priority_Background;
		case RPG::SavePicture::MapLayer_tilemap_below:
			return Priority_TilesetBelow;
		case RPG::SavePicture::MapLayer_events_below:
			return Priority_EventsBelow;
		case RPG::SavePicture::MapLayer_events_same_as_player:
			return Priority_Player;
		case RPG::SavePicture::MapLayer_tilemap_above:
			return Priority_TilesetAbove;
		case RPG::SavePicture::MapLayer_events_above:
			return Priority_EventsAbove;
		case RPG::SavePicture::MapLayer_weather:
			return Priority_PictureNew;
		case RPG::SavePicture::MapLayer_animations:
			return Priority_BattleAnimation;
		case RPG::SavePicture::MapLayer_windows:
			return Priority_Window;
		case RPG::SavePicture::MapLayer_timers:
			return Priority_Timer;
		default:
			return 0;
	}
}

int Drawable::GetPriorityForBattleLayer(int which) {
	switch (which) {
		case RPG::SavePicture::BattleLayer_background:
			return Priority_Background;
		case RPG::SavePicture::BattleLayer_battlers_and_animations:
			return Priority_Battler;
		case RPG::SavePicture::BattleLayer_weather:
			return Priority_PictureNew;
		case RPG::SavePicture::BattleLayer_windows_and_status:
			return Priority_Window;
		case RPG::SavePicture::BattleLayer_timers:
			return Priority_Timer;
		default:
			return 0;
	}
}
