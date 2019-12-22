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
#include "drawable_list.h"

// Constructor
Spriteset_Map::Spriteset_Map() {
	tilemap.reset(new Tilemap());
	tilemap->SetWidth(Game_Map::GetWidth());
	tilemap->SetHeight(Game_Map::GetHeight());

	panorama.reset(new Plane());
	panorama->SetZ(Priority_Background);

	ChipsetUpdated();

	need_x_clone = Game_Map::LoopHorizontal();
	need_y_clone = Game_Map::LoopVertical();

	for (Game_Event& ev : Game_Map::GetEvents()) {
		CreateSprite(&ev, need_x_clone, need_y_clone);
	}

	CreateAirshipShadowSprite(need_x_clone, need_y_clone);

	CreateSprite(Main_Data::game_player.get(), need_x_clone, need_y_clone);

	timer1.reset(new Sprite_Timer(0));
	timer2.reset(new Sprite_Timer(1));

	screen.reset(new Screen());
	frame.reset(new Frame());

	Update();
}

// Update
void Spriteset_Map::Update() {
	Tone new_tone = Main_Data::game_screen->GetTone();

	tilemap->SetOx(Game_Map::GetDisplayX() / (SCREEN_TILE_SIZE / TILE_SIZE));
	tilemap->SetOy(Game_Map::GetDisplayY() / (SCREEN_TILE_SIZE / TILE_SIZE));
	tilemap->SetTone(new_tone);
	tilemap->Update();

	for (size_t i = 0; i < character_sprites.size(); i++) {
		character_sprites[i]->Update();
		character_sprites[i]->SetTone(new_tone);
	}

	std::string name = Game_Map::Parallax::GetName();
	if (name != panorama_name) {
		panorama_name = name;
		if (name.empty()) {
			panorama->SetBitmap(BitmapRef());
		} else {
			FileRequestAsync *request = AsyncHandler::RequestFile("Panorama", panorama_name);
			request->SetGraphicFile(true);
			panorama_request_id = request->Bind(&Spriteset_Map::OnPanoramaSpriteReady, this);
			request->Start();
		}
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
			CreateSprite(vehicle, need_x_clone, need_y_clone);
		}
	}

	for (auto& shadow : airship_shadows) {
		shadow->SetTone(new_tone);
		shadow->Update();
	}

	timer1->Update();
	timer2->Update();
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
		request->SetGraphicFile(true);
		request->Start();
	}
	else {
		OnTilemapSpriteReady(NULL);
	}
}

void Spriteset_Map::SystemGraphicUpdated() {
	for (auto& shadow : airship_shadows) {
		shadow->RecreateShadow();
	}
}

void Spriteset_Map::SubstituteDown(int old_id, int new_id) {
	int num_subst = Game_Map::SubstituteDown(old_id, new_id);
	if (num_subst) {
		tilemap->OnSubstituteDown();
	}
}

void Spriteset_Map::SubstituteUp(int old_id, int new_id) {
	int num_subst = Game_Map::SubstituteUp(old_id, new_id);
	if (num_subst) {
		tilemap->OnSubstituteUp();
	}
}

bool Spriteset_Map::RequireBackground(const DrawableList& drawable_list) {
	// Speed optimisation:
	// When there is nothing below the tilemap it can be drawn opaque (faster)
	tilemap->SetFastBlitDown(false);

	if (!panorama_name.empty()) {
		// Map has a panorama -> No opaque tilemap blit possible
		// but the panorama is drawn opaque -> clearing the screen is not needed
		return false;
	}

	for (const Drawable* d : drawable_list) {
		if (d->GetZ() > Priority_Background) {
			if (d->GetZ() < Priority_TilesetBelow) {
				// There is a picture below the tilemap -> No opaque tilemap blit possible
				return true;
			} else {
				tilemap->SetFastBlitDown(true);
				return true;
			}
		}
	}

	// shouldn't happen
	assert(false);
	return true;
}

void Spriteset_Map::CreateSprite(Game_Character* character, bool create_x_clone, bool create_y_clone) {
	using CloneType = Sprite_Character::CloneType;

	character_sprites.push_back(std::make_shared<Sprite_Character>(character));
	if (create_x_clone) {
		character_sprites.push_back(std::make_shared<Sprite_Character>(character, CloneType::XClone));
	}
	if (create_y_clone) {
		character_sprites.push_back(std::make_shared<Sprite_Character>(character, CloneType::YClone));
	}
	if (create_x_clone && create_y_clone) {
		character_sprites.push_back(std::make_shared<Sprite_Character>(character,
			(CloneType)(CloneType::XClone | CloneType::YClone)));
	}
}

void Spriteset_Map::CreateAirshipShadowSprite(bool create_x_clone, bool create_y_clone) {
	using CloneType = Sprite_AirshipShadow::CloneType;

	airship_shadows.push_back(std::make_shared<Sprite_AirshipShadow>());
	if (create_x_clone) {
		airship_shadows.push_back(std::make_shared<Sprite_AirshipShadow>(CloneType::XClone));
	}
	if (create_y_clone) {
		airship_shadows.push_back(std::make_shared<Sprite_AirshipShadow>(CloneType::YClone));
	}
	if (create_x_clone && create_y_clone) {
		airship_shadows.push_back(std::make_shared<Sprite_AirshipShadow>(
			(CloneType)(CloneType::XClone | CloneType::YClone)));
	}
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
}

void Spriteset_Map::OnPanoramaSpriteReady(FileRequestResult* result) {
	BitmapRef panorama_bmp = Cache::Panorama(result->file);
	panorama->SetBitmap(panorama_bmp);
	Game_Map::Parallax::Initialize(panorama_bmp->GetWidth(), panorama_bmp->GetHeight());
}
