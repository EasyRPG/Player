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
#include "dynrpg.h"
#include "game_map.h"
#include "main_data.h"
#include "sprite_airshipshadow.h"
#include "sprite_character.h"
#include "game_character.h"
#include "game_player.h"
#include "game_vehicle.h"
#include "game_screen.h"
#include "bitmap.h"
#include "player.h"
#include "drawable_list.h"

Spriteset_Map::Spriteset_Map() {
	panorama = std::make_unique<Plane>();
	panorama->SetZ(Priority_Background);

	timer1 = std::make_unique<Sprite_Timer>(0);
	timer2 = std::make_unique<Sprite_Timer>(1);

	screen = std::make_unique<Screen>();

	if (Player::IsRPG2k3()) {
		frame = std::make_unique<Frame>();
	}

	ParallaxUpdated();

	Refresh();

	Update();
}

void Spriteset_Map::Refresh() {
	CalculateMapRenderOffset();

	tilemap = std::make_unique<Tilemap>();
	tilemap->SetWidth(Game_Map::GetTilesX());
	tilemap->SetHeight(Game_Map::GetTilesY());
	tilemap->SetRenderOx(map_render_ox);
	tilemap->SetRenderOy(map_render_oy);

	airship_shadows.clear();
	character_sprites.clear();

	ChipsetUpdated();

	need_x_clone = Game_Map::LoopHorizontal();
	need_y_clone = Game_Map::LoopVertical();

	for (Game_Event& ev : Game_Map::GetEvents()) {
		CreateSprite(&ev, need_x_clone, need_y_clone);
	}

	CreateAirshipShadowSprite(need_x_clone, need_y_clone);

	CreateSprite(Main_Data::game_player.get(), need_x_clone, need_y_clone);

	for (bool& v: vehicle_loaded) {
		v = false;
	}
}

// Update
void Spriteset_Map::Update() {
	Tone new_tone = Main_Data::game_screen->GetTone();

	tilemap->SetOx(Game_Map::GetDisplayX() / (SCREEN_TILE_SIZE / TILE_SIZE));
	tilemap->SetOy(Game_Map::GetDisplayY() / (SCREEN_TILE_SIZE / TILE_SIZE));
	tilemap->SetTone(new_tone);

	for (const auto& character_sprite : character_sprites) {
		character_sprite->Update();
		character_sprite->SetTone(new_tone);
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

	DynRpg::Update();
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

	for (auto& sprite: character_sprites) {
		sprite->ChipsetUpdated();
	}
}

void Spriteset_Map::ParallaxUpdated() {
	std::string name = Game_Map::Parallax::GetName();
	if (name != panorama_name) {
		panorama_name = name;
		if (!name.empty()) {
			FileRequestAsync* request = AsyncHandler::RequestFile("Panorama", panorama_name);
			request->SetGraphicFile(true);
			request->SetImportantFile(true);
			panorama_request_id = request->Bind(&Spriteset_Map::OnPanoramaSpriteReady, this);
			request->Start();
		}
	}

	if (name.empty()) {
		panorama->SetBitmap(BitmapRef());
		Game_Map::Parallax::Initialize(0, 0);
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

bool Spriteset_Map::RequireClear(DrawableList& drawable_list) {
	if (drawable_list.empty()) {
		return true;
	}

	// When using a custom resolution that is not divisible by 16, clear to avoid
	// artifacts at the borders
	if (Player::screen_width % TILE_SIZE != 0 || Player::screen_height % TILE_SIZE != 0) {
		return true;
	}

	// Speed optimisation:
	// When there is nothing below the tilemap it can be drawn opaque (faster)
	tilemap->SetFastBlitDown(false);

	if (!panorama_name.empty()) {
		// Map has a panorama -> No opaque tilemap blit possible
		// but the panorama is drawn opaque -> clearing the screen is not needed
		return false;
	}

	// The list is about to be drawn, so we can just sort it now if needed.
	if (drawable_list.IsDirty()) {
		drawable_list.Sort();
	}

	// Only if there is nothing below the tileset, can we do fast blitting.
	if ((*drawable_list.begin())->GetZ() >= Priority_TilesetBelow) {
		tilemap->SetFastBlitDown(true);
	}

	return true;
}

void Spriteset_Map::CreateSprite(Game_Character* character, bool create_x_clone, bool create_y_clone) {
	using CloneType = Sprite_Character::CloneType;

	auto add_sprite = [&](auto&& chara) {
		chara->SetRenderOx(map_render_ox);
		chara->SetRenderOy(map_render_oy);
		character_sprites.push_back(std::forward<decltype(chara)>(chara));
	};

	add_sprite(std::make_unique<Sprite_Character>(character));
	if (create_x_clone) {
		add_sprite(std::make_unique<Sprite_Character>(character, CloneType::XClone));
	}
	if (create_y_clone) {
		add_sprite(std::make_unique<Sprite_Character>(character, CloneType::YClone));
	}
	if (create_x_clone && create_y_clone) {
		add_sprite(std::make_unique<Sprite_Character>(character,
			(CloneType)(CloneType::XClone | CloneType::YClone)));
	}
}

void Spriteset_Map::CreateAirshipShadowSprite(bool create_x_clone, bool create_y_clone) {
	using CloneType = Sprite_AirshipShadow::CloneType;

	auto add_sprite = [&](auto&& chara) {
		chara->SetRenderOx(map_render_ox);
		chara->SetRenderOy(map_render_oy);
		airship_shadows.push_back(std::forward<decltype(chara)>(chara));
	};

	add_sprite(std::make_unique<Sprite_AirshipShadow>());
	if (create_x_clone) {
		add_sprite(std::make_unique<Sprite_AirshipShadow>(CloneType::XClone));
	}
	if (create_y_clone) {
		add_sprite(std::make_unique<Sprite_AirshipShadow>(CloneType::YClone));
	}
	if (create_x_clone && create_y_clone) {
		add_sprite(std::make_unique<Sprite_AirshipShadow>(
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
	CalculatePanoramaRenderOffset();
}

void Spriteset_Map::CalculateMapRenderOffset() {
	map_render_ox = 0;
	map_render_oy = 0;
	map_tiles_x = 0;
	map_tiles_y = 0;

	panorama->SetRenderOx(0);
	panorama->SetRenderOy(0);
	screen->SetViewport(Rect());

	if (Player::game_config.fake_resolution.Get()) {
		// Resolution hack for tiles and sprites
		// Smallest possible map. Smaller maps are hacked
		map_tiles_x = std::max<int>(Game_Map::GetTilesX(), 20) * TILE_SIZE;
		map_tiles_y = std::max<int>(Game_Map::GetTilesY(), 15) * TILE_SIZE;

		if (map_tiles_x < Player::screen_width) {
			map_render_ox = (Player::screen_width - map_tiles_x) / 2;
		}
		if (map_tiles_y < Player::screen_height) {
			map_render_oy = (Player::screen_height - map_tiles_y) / 2;
		}

		CalculatePanoramaRenderOffset();

		screen->SetViewport({map_render_ox, map_render_oy, map_tiles_x, map_tiles_y});
	}
}

void Spriteset_Map::CalculatePanoramaRenderOffset() {
	// Resolution hack for Panorama
	if (Player::game_config.fake_resolution.Get()) {
		if (Game_Map::Parallax::FakeXPosition()) {
			panorama->SetRenderOx((Player::screen_width - SCREEN_TARGET_WIDTH) / 2);
		}
		if (Game_Map::Parallax::FakeYPosition()) {
			panorama->SetRenderOy((Player::screen_height - SCREEN_TARGET_HEIGHT) / 2);
		}
	}
}
