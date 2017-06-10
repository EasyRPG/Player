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

// Headers
#include "cache.h"
#include "bitmap.h"
#include "game_map.h"
#include "game_player.h"
#include "game_system.h"
#include "main_data.h"
#include "sprite_airshipshadow.h"
#include <string>

Sprite_AirshipShadow::Sprite_AirshipShadow() {
	SetBitmap(Bitmap::Create(16,16));

	SetOx(TILE_SIZE/2);
	SetOy(TILE_SIZE);

	RecreateShadow();
}

// Draws the two shadow sprites to a single intermediate bitmap to be blit to the map
// Needs to be recalled when the System graphic changes
void Sprite_AirshipShadow::RecreateShadow() {
	GetBitmap()->Clear();

	std::string system_name = Game_System::GetSystemName();
	if (system_name.empty()) return;

	BitmapRef system = Cache::System(system_name);

	// Offset of the shadow in the System graphic as per
	// https://wiki.easyrpg.org/development/technical-details/system-graphics
	// TODO 26% opacity looks okay, but isn't accurate to RPG_RT
	Opacity opacity = Opacity(0.26 * 255);
	GetBitmap()->Blit(0, 0, *system, Rect(128,32,16,16), opacity);
	GetBitmap()->Blit(0, 0, *system, Rect(128+16,32,16,16), opacity);
}

void Sprite_AirshipShadow::Update() {
	Sprite::Update();

	if (!Main_Data::game_player->InAirship()) {
		SetVisible(false);
		return;
	}
	SetVisible(true);

	Game_Vehicle* airship = Game_Map::GetVehicle(Game_Vehicle::Airship);

	const int altitude = airship->GetAltitude();
	const int max_altitude = TILE_SIZE;
	const double opacity = (double)altitude / max_altitude;
	SetOpacity(opacity * 255);

	SetX(Main_Data::game_player->GetScreenX());
	SetY(Main_Data::game_player->GetScreenY());
	// Synchronized with airship priority
	SetZ(airship->GetScreenZ());
}
