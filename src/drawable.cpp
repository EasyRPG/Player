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
#include <lcf/rpg/savepicture.h>
#include "drawable_mgr.h"

Drawable::~Drawable() {
	DrawableMgr::Remove(this);
}

void Drawable::SetZ(int nz) {
	if (_z != nz) DrawableMgr::OnUpdateZ(this);
	_z = nz;
}

int Drawable::GetPriorityForMapLayer(int which) {
	switch (which) {
		case lcf::rpg::SavePicture::MapLayer_parallax:
			return Priority_Background;
		case lcf::rpg::SavePicture::MapLayer_tilemap_below:
			return Priority_TilesetBelow;
		case lcf::rpg::SavePicture::MapLayer_events_below:
			return Priority_EventsBelow;
		case lcf::rpg::SavePicture::MapLayer_events_same_as_player:
			return Priority_Player;
		case lcf::rpg::SavePicture::MapLayer_tilemap_above:
			return Priority_TilesetAbove;
		case lcf::rpg::SavePicture::MapLayer_events_above:
			return Priority_EventsFlying;
		case lcf::rpg::SavePicture::MapLayer_weather:
			return Priority_PictureNew;
		case lcf::rpg::SavePicture::MapLayer_animations:
			return Priority_BattleAnimation;
		case lcf::rpg::SavePicture::MapLayer_windows:
			return Priority_Window;
		case lcf::rpg::SavePicture::MapLayer_timers:
			return Priority_Timer;
		default:
			return 0;
	}
}

int Drawable::GetPriorityForBattleLayer(int which) {
	switch (which) {
		case lcf::rpg::SavePicture::BattleLayer_background:
			return Priority_Background;
		case lcf::rpg::SavePicture::BattleLayer_battlers_and_animations:
			return Priority_Battler;
		case lcf::rpg::SavePicture::BattleLayer_weather:
			return Priority_PictureNew;
		case lcf::rpg::SavePicture::BattleLayer_windows_and_status:
			return Priority_Window;
		case lcf::rpg::SavePicture::BattleLayer_timers:
			return Priority_Timer;
		default:
			return 0;
	}
}
