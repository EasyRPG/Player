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
#include "player.h"

// Constructor
Spriteset_Map::Spriteset_Map() {
	tilemap.reset(new Tilemap());
	tilemap->SetWidth(Game_Map::GetWidth());
	tilemap->SetHeight(Game_Map::GetHeight());

	panorama.reset(new Plane());
	panorama->SetZ(-1000);

	ChipsetUpdated();

	for (Game_Event& ev : Game_Map::GetEvents()) {
		character_sprites.push_back(std::make_shared<Sprite_Character>(&ev));
	}

	airship_shadow.reset(new Sprite_AirshipShadow());

	character_sprites.push_back
		(std::make_shared<Sprite_Character>(Main_Data::game_player.get()));

	timer1.reset(new Sprite_Timer(0));
	timer2.reset(new Sprite_Timer(1));

	screen.reset(new Screen());
	weather.reset(new Weather());
	frame.reset(new Frame());

	Update();
}

// Update
void Spriteset_Map::Update() {
	Tone new_tone = Main_Data::game_screen->GetTone();

	if (new_tone != last_tone) {
		// Could be a gradient change, just updating the display is faster
		screen->SetTone(new_tone);
		last_tone = new_tone;

		// Normal tone for all graphics
		new_tone = Tone();
	} else {
		// Not a gradient change, use the cached Tone graphics instead of
		// recalculating the screen tone
		screen->SetTone(Tone());
	}

	tilemap->SetOx(Game_Map::GetDisplayX() / (SCREEN_TILE_WIDTH / TILE_SIZE));
	tilemap->SetOy(Game_Map::GetDisplayY() / (SCREEN_TILE_WIDTH / TILE_SIZE));
	tilemap->SetTone(new_tone);
	tilemap->Update();

	for (size_t i = 0; i < character_sprites.size(); i++) {
		character_sprites[i]->Update();
		character_sprites[i]->SetTone(new_tone);
	}

	std::string name = Game_Map::Parallax::GetName();
	if (name != panorama_name) {
		panorama_name = name;
		FileRequestAsync* request = AsyncHandler::RequestFile("Panorama", panorama_name);
		panorama_request_id = request->Bind(&Spriteset_Map::OnPanoramaSpriteReady, this);
		request->Start();
	}
	panorama->SetOx(Game_Map::Parallax::GetX());
	panorama->SetOy(Game_Map::Parallax::GetY());
	panorama->SetTone(new_tone);

	Game_Vehicle* vehicle;
	int map_id = Game_Map::GetMapId();
	for (int i = 1; i <= 3; ++i) {
		vehicle = Game_Map::GetVehicle((Game_Vehicle::Type) i);

		if (!vehicle_loaded[i - 1] && vehicle->GetMapId() == map_id) {
			vehicle_loaded[i - 1] = true;
			character_sprites.push_back(std::make_shared<Sprite_Character>(vehicle));
		}
	}

	airship_shadow->Update();

	timer1->Update();
	timer2->Update();

	weather->SetTone(new_tone);
}

// Finds the sprite for a specific character
Sprite_Character* Spriteset_Map::FindCharacter(Game_Character* character) const
{
	std::vector<std::shared_ptr<Sprite_Character> >::const_iterator it;
	for (it = character_sprites.begin(); it != character_sprites.end(); ++it) {
		Sprite_Character* sprite = it->get();
		if (sprite->GetCharacter() == character)
			return sprite;
	}
	return NULL;
}

void Spriteset_Map::ChipsetUpdated() {
	if (!Game_Map::GetChipsetName().empty()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("ChipSet", Game_Map::GetChipsetName());
		tilemap_request_id = request->Bind(&Spriteset_Map::OnTilemapSpriteReady, this);
		request->SetImportantFile(true);
		request->Start();
	}
	else {
		OnTilemapSpriteReady(NULL);
	}
}

void Spriteset_Map::SystemGraphicUpdated() {
	airship_shadow->RecreateShadow();
}

void Spriteset_Map::SubstituteDown(int old_id, int new_id) {
	Game_Map::SubstituteDown(old_id, new_id);
	tilemap->SubstituteDown(old_id, new_id);
}

void Spriteset_Map::SubstituteUp(int old_id, int new_id) {
	Game_Map::SubstituteUp(old_id, new_id);
	tilemap->SubstituteUp(old_id, new_id);
}

void Spriteset_Map::OnTilemapSpriteReady(FileRequestResult*) {
	if (!Game_Map::GetChipsetName().empty()) {
		tilemap->SetChipset(Cache::Chipset(Game_Map::GetChipsetName()));
	}
	else {
		tilemap->SetChipset(Bitmap::Create(480, 256));
	}

	tilemap->SetMapDataDown(Game_Map::GetMapDataDown());
	tilemap->SetMapDataUp(Game_Map::GetMapDataUp());
	tilemap->SetPassableDown(Game_Map::GetPassagesDown());
	tilemap->SetPassableUp(Game_Map::GetPassagesUp());
	tilemap->SetAnimationType(Game_Map::GetAnimationType());
	tilemap->SetAnimationSpeed(Game_Map::GetAnimationSpeed());

	tilemap->SetFastBlitDown(!panorama->GetBitmap());
}

void Spriteset_Map::OnPanoramaSpriteReady(FileRequestResult* result) {
	BitmapRef panorama_bmp = Cache::Panorama(result->file);
	panorama->SetBitmap(panorama_bmp);
	Game_Map::Parallax::Initialize(panorama_bmp->GetWidth(), panorama_bmp->GetHeight());

	tilemap->SetFastBlitDown(false);
}
