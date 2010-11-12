/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "spriteset_map.h"
#include "cache.h"
#include "game_map.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Spriteset_Map::Spriteset_Map() {
	tilemap = new Tilemap();
	tilemap->SetWidth(Main_Data::game_map->GetWidth());
	tilemap->SetHeight(Main_Data::game_map->GetHeight());
	tilemap->SetChipset(Cache::Chipset(Main_Data::game_map->chipset_name));
	tilemap->SetPassableDown(Main_Data::game_map->passages_down);
	tilemap->SetPassableUp(Main_Data::game_map->passages_up);
	tilemap->SetMapDataDown(Main_Data::game_map->GetMapDataDown());
	tilemap->SetMapDataUp(Main_Data::game_map->GetMapDataUp());
	panorama = new Plane();
	panorama->SetZ(-1000);
	fog = new Plane();
	fog->SetZ(3000);
	/*for (int i = 0; i < Main_Data::game_map->events.size; i++) {
		Sprite_Character* sprite = new Sprite_Character();
		character_sprites.push(sprite);
	}
	Sprite_Character* player = new Sprite_Character();
	character_sprites.push(player);
	weather = new Weather();
	for (int i = 0; i < 50; i++) {
		Sprite_Picture* sprite = new Sprite_Picture();
		picture_sprites.push(sprite);
	}
	timer_sprite = new Sprite_Timer();*/
	Update();
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Spriteset_Map::~Spriteset_Map() {
	delete tilemap;
	delete panorama;
	delete fog;
	/*for (int i = 0; i < character_sprites.size(); i++) {
		delete character_sprites[i];
	}
	for (int i = 0; i < picture_sprites.size(); i++) {
		delete picture_sprites[i];
	}
	delete weather;
	delete timer_sprite;*/
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Spriteset_Map::Update() {
	tilemap->SetOx(Main_Data::game_map->display_x / 8);
	tilemap->SetOy(Main_Data::game_map->display_y / 8);
	tilemap->Update();
}
