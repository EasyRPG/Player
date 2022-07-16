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

void Drawable::SetZ(Z_t nz) {
	if (_z != nz) DrawableMgr::OnUpdateZ(this);
	_z = nz;
}

Drawable::Z_t Drawable::GetPriorityForMapLayer(int which) {
	Z_t layer = 0;

	switch (which) {
		case lcf::rpg::SavePicture::MapLayer_parallax:
			layer = Priority_Background;
			break;
		case lcf::rpg::SavePicture::MapLayer_tilemap_below:
			layer = Priority_TilesetBelow;
			break;
		case lcf::rpg::SavePicture::MapLayer_events_below:
			layer = Priority_EventsBelow;
			break;
		case lcf::rpg::SavePicture::MapLayer_events_same_as_player:
			layer = Priority_Player;
			break;
		case lcf::rpg::SavePicture::MapLayer_tilemap_above:
			layer = Priority_TilesetAbove;
			break;
		case lcf::rpg::SavePicture::MapLayer_events_above:
			layer = Priority_EventsFlying;
			break;
		case lcf::rpg::SavePicture::MapLayer_weather:
			layer = Priority_PictureNew;
			break;
		case lcf::rpg::SavePicture::MapLayer_animations:
			layer = Priority_BattleAnimation;
			break;
		case lcf::rpg::SavePicture::MapLayer_windows:
			layer = Priority_Window;
			break;
		case lcf::rpg::SavePicture::MapLayer_timers:
			layer = Priority_Timer;
			break;
		default:
			return layer;
	}

	return layer + (1ULL << z_offset);
}

Drawable::Z_t Drawable::GetPriorityForBattleLayer(int which) {
	Z_t layer = 0;

	switch (which) {
		case lcf::rpg::SavePicture::BattleLayer_background:
			layer = Priority_Background;
			break;
		case lcf::rpg::SavePicture::BattleLayer_battlers_and_animations:
			layer = Priority_Battler;
			break;
		case lcf::rpg::SavePicture::BattleLayer_weather:
			layer = Priority_PictureNew;
			break;
		case lcf::rpg::SavePicture::BattleLayer_windows_and_status:
			layer = Priority_Window;
			break;
		case lcf::rpg::SavePicture::BattleLayer_timers:
			layer = Priority_Timer;
			break;
		default:
			return layer;
	}

	return layer + (1ULL << z_offset);
}
