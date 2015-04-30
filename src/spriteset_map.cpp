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
#include "spriteset_map.h"
#include "cache.h"
#include "game_map.h"
#include "main_data.h"
#include "sprite_airshipshadow.h"
#include "sprite_character.h"
#include "game_character.h"
#include "game_player.h"
#include "game_vehicle.h"
#include "bitmap.h"

// Constructor
Spriteset_Map::Spriteset_Map() {
	tilemap.SetWidth(Game_Map::GetWidth());
	tilemap.SetHeight(Game_Map::GetHeight());
	ChipsetUpdated();
	tilemap.SetMapDataDown(Game_Map::GetMapDataDown());
	tilemap.SetMapDataUp(Game_Map::GetMapDataUp());

	panorama.SetZ(-1000);

	tEventHash events = Game_Map::GetEvents();
	for (tEventHash::iterator i = events.begin(); i != events.end(); ++i) {
		character_sprites.push_back(EASYRPG_MAKE_SHARED<Sprite_Character>(i->second.get()));
	}

	Game_Vehicle* vehicle;
	for (int i = 1; i <= 3; ++i) {
		vehicle = Game_Map::GetVehicle((Game_Vehicle::Type) i);
		character_sprites.push_back(EASYRPG_MAKE_SHARED<Sprite_Character>(vehicle));
	}
	airship_shadow.reset(new Sprite_AirshipShadow());

	character_sprites.push_back
		(EASYRPG_MAKE_SHARED<Sprite_Character>(Main_Data::game_player.get()));

	timer1.reset(new Sprite_Timer(0));
	timer2.reset(new Sprite_Timer(1));

	Update();
}

// Update
void Spriteset_Map::Update() {
	tilemap.SetOx(Game_Map::GetDisplayX() / (SCREEN_TILE_WIDTH / TILE_SIZE));
	tilemap.SetOy(Game_Map::GetDisplayY() / (SCREEN_TILE_WIDTH / TILE_SIZE));
	tilemap.Update();
	for (size_t i = 0; i < character_sprites.size(); i++) {
		character_sprites[i]->Update();
	}
	const std::string& name = Game_Map::GetParallaxName();
	if (name != panorama_name) {
		panorama_name = name;
		BitmapRef panorama_bmp = Cache::Panorama(panorama_name); // TODO
		Game_Map::SetParallaxSize(panorama_bmp->GetWidth(), panorama_bmp->GetHeight());
		panorama.SetBitmap(panorama_bmp);
		Game_Map::InitializeParallax();
	}
	panorama.SetOx(Game_Map::GetParallaxX());
	panorama.SetOy(Game_Map::GetParallaxY());

	airship_shadow->Update();

	timer1->Update();
	timer2->Update();
}

// Finds the sprite for a specific character
Sprite_Character* Spriteset_Map::FindCharacter(Game_Character* character) const
{
	std::vector<EASYRPG_SHARED_PTR<Sprite_Character> >::const_iterator it;
	for (it = character_sprites.begin(); it != character_sprites.end(); ++it) {
		Sprite_Character* sprite = it->get();
		if (sprite->GetCharacter() == character)
			return sprite;
	}
	return NULL;
}

void Spriteset_Map::ChipsetUpdated() {
	tilemap.SetChipset(Cache::Chipset(Game_Map::GetChipsetName())); // TODO
	tilemap.SetPassableDown(Game_Map::GetPassagesDown());
	tilemap.SetPassableUp(Game_Map::GetPassagesUp());
	tilemap.SetAnimationType(Game_Map::GetAnimationType());
	tilemap.SetAnimationSpeed(Game_Map::GetAnimationSpeed());
}

void Spriteset_Map::SystemGraphicUpdated() {
	airship_shadow->RecreateShadow();
}

void Spriteset_Map::SubstituteDown(int old_id, int new_id) {
	Game_Map::SubstituteDown(old_id, new_id);
	tilemap.SubstituteDown(old_id, new_id);
}

void Spriteset_Map::SubstituteUp(int old_id, int new_id) {
	Game_Map::SubstituteUp(old_id, new_id);
	tilemap.SubstituteUp(old_id, new_id);
}
