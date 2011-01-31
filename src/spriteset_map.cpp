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
	tilemap->SetWidth(Game_Map::GetWidth());
	tilemap->SetHeight(Game_Map::GetHeight());
	tilemap->SetChipset(Cache::Chipset(Game_Map::GetChipsetName()));
	tilemap->SetPassableDown(Game_Map::GetPassagesDown());
	tilemap->SetPassableUp(Game_Map::GetPassagesUp());
	tilemap->SetMapDataDown(Game_Map::GetMapDataDown());
	tilemap->SetMapDataUp(Game_Map::GetMapDataUp());
	panorama = new Plane();
	panorama->SetZ(-1000);

	tEventHash events = Game_Map::GetEvents();
	for (tEventHash::iterator i = events.begin(); i != events.end(); i++) {
		Sprite_Character* sprite = new Sprite_Character(i->second);
		character_sprites.push_back(sprite);
	}
	Sprite_Character* player = new Sprite_Character((Game_Character*)Main_Data::game_player);
	character_sprites.push_back(player);
	/*weather = new Weather();
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
	for (size_t i = 0; i < character_sprites.size(); i++) {
		delete character_sprites[i];
	}
	/*for (int i = 0; i < picture_sprites.size(); i++) {
		delete picture_sprites[i];
	}
	delete weather;
	delete timer_sprite;*/
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Spriteset_Map::Update() {
	tilemap->SetOx(Game_Map::GetDisplayX() / 8);
	tilemap->SetOy(Game_Map::GetDisplayY() / 8);
	tilemap->Update();
	for (size_t i = 0; i < character_sprites.size(); i++) {
		character_sprites[i]->Update();
	}
	const std::string& name = Game_Map::GetParallaxName();
	if (name != panorama_name) {
		panorama_name = name;
		panorama->SetBitmap(Cache::Panorama(panorama_name));
	}
	panorama->SetOx(Game_Map::GetParallaxX());
	panorama->SetOy(Game_Map::GetParallaxY());
}

////////////////////////////////////////////////////////////
/// Find the sprite for a specific character
////////////////////////////////////////////////////////////
Sprite_Character* Spriteset_Map::FindCharacter(Game_Character* character) const
{
	std::vector<Sprite_Character*>::const_iterator it;
	for (it = character_sprites.begin(); it != character_sprites.end(); it++) {
		Sprite_Character* sprite = *it;
		if (sprite->GetCharacter() == character)
			return sprite;
	}
	return NULL;
}

void Spriteset_Map::ChipsetUpdated() {
	tilemap->SetChipset(Cache::Chipset(Game_Map::GetChipsetName()));
	tilemap->SetPassableDown(Game_Map::GetPassagesDown());
	tilemap->SetPassableUp(Game_Map::GetPassagesUp());
}

void Spriteset_Map::SubstituteDown(int old_id, int new_id) {
	Game_Map::SubstituteDown(old_id, new_id);
	tilemap->SubstituteDown(old_id, new_id);
}

void Spriteset_Map::SubstituteUp(int old_id, int new_id) {
	Game_Map::SubstituteUp(old_id, new_id);
	tilemap->SubstituteUp(old_id, new_id);
}
