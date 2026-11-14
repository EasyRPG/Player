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
#include "sprite_character.h"
#include "cache.h"
#include "game_map.h"
#include "bitmap.h"
#include "output.h"
#include "player.h"

Sprite_Character::Sprite_Character(Game_Character* character, int x_offset, int y_offset) :
	character(character),
	tile_id(-1),
	character_index(0),
	chara_width(0),
	chara_height(0),
	x_offset(x_offset),
	y_offset(y_offset) {

	Update();
}

void Sprite_Character::Draw(Bitmap &dst) {
	if (UsesCharset()) {
		int row = character->GetFacing();
		auto frame = character->GetAnimFrame();
		if (frame >= lcf::rpg::EventPage::Frame_middle2) frame = lcf::rpg::EventPage::Frame_middle;
		SetSrcRect({frame * chara_width, row * chara_height, chara_width, chara_height});
	}

	SetFlashEffect(character->GetFlashColor());

	SetOpacity(character->GetOpacity());

	SetX(character->GetScreenX() + x_offset);
	SetY(character->GetScreenY() + y_offset);

	int bush_split = 4 - character->GetBushDepth();
	SetBushDepth(bush_split > 3 ? 0 : GetHeight() / bush_split);

	Sprite::Draw(dst);

	// --- DEBUG COLLISION BOX OVERLAY ---
	if (character && character->IsCollisionDebugEnabled()) {
		int min_dx, max_dx, min_dy, max_dy;
		character->GetTileOffsets(min_dx, max_dx, min_dy, max_dy);

		std::vector<Game_Character::TileOffset> null_zones;
		character->GetNullZones(null_zones);

		int base_tile_screen_x = character->GetScreenX() - (TILE_SIZE / 2);
		int base_tile_screen_y = character->GetScreenY() - TILE_SIZE;

		for (int dy = min_dy; dy <= max_dy; ++dy) {
			for (int dx = min_dx; dx <= max_dx; ++dx) {
				bool is_null = false;
				for (const auto& nz : null_zones) {
					if (nz.dx == dx && nz.dy == dy) {
						is_null = true;
						break;
					}
				}

				int tile_screen_x = base_tile_screen_x + (dx * TILE_SIZE);
				int tile_screen_y = base_tile_screen_y + (dy * TILE_SIZE);
				Rect tile_rect(tile_screen_x, tile_screen_y, TILE_SIZE, TILE_SIZE);

				if (is_null) {
					// Draw null zone as a faint blue/gray or dashed outline
					dst.FillRect(tile_rect, Color(0, 120, 255, 30));
				} else {
					// Draw solid collision tile
					dst.FillRect(tile_rect, Color(255, 0, 0, 80));
					dst.FillRect(Rect(tile_rect.x, tile_rect.y, tile_rect.width, 1), Color(255, 255, 255, 255));
					dst.FillRect(Rect(tile_rect.x, tile_rect.y + tile_rect.height - 1, tile_rect.width, 1), Color(255, 255, 255, 255));
					dst.FillRect(Rect(tile_rect.x, tile_rect.y, 1, tile_rect.height), Color(255, 255, 255, 255));
					dst.FillRect(Rect(tile_rect.x + tile_rect.width - 1, tile_rect.y, 1, tile_rect.height), Color(255, 255, 255, 255));
				}
			}
		}
	}
}

void Sprite_Character::Update() {
	if (tile_id != character->GetTileId() ||
		character_name != character->GetSpriteName() ||
		character_index != character->GetSpriteIndex() ||
		refresh_bitmap
	) {
		tile_id = character->GetTileId();
		character_name = character->GetSpriteName();
		character_index = character->GetSpriteIndex();
		refresh_bitmap = false;

		if (UsesCharset()) {
			FileRequestAsync* char_request = AsyncHandler::RequestFile("CharSet", character_name);
			char_request->SetGraphicFile(true);
			request_id = char_request->Bind(&Sprite_Character::OnCharSpriteReady, this);
			char_request->Start();
		} else {
			const auto chipset_name = Game_Map::GetChipsetName();
			if (chipset_name.empty()) {
				OnTileSpriteReady(nullptr);
			} else {
				FileRequestAsync *tile_request = AsyncHandler::RequestFile("ChipSet", Game_Map::GetChipsetName());
				tile_request->SetGraphicFile(true);
				request_id = tile_request->Bind(&Sprite_Character::OnTileSpriteReady, this);
				tile_request->Start();
			}
		}
	}

	SetVisible(character->IsVisible());
	SetZ(character->GetScreenZ(x_offset, y_offset));
}

Game_Character* Sprite_Character::GetCharacter() {
	return character;
}

void Sprite_Character::SetCharacter(Game_Character* new_character) {
	character = new_character;
}

bool Sprite_Character::UsesCharset() const {
	return !character_name.empty();
}

void Sprite_Character::OnTileSpriteReady(FileRequestResult*) {
	const auto chipset = Game_Map::GetChipsetName();

	BitmapRef tile;
	if (!chipset.empty()) {
		tile = Cache::Tile(Game_Map::GetChipsetName(), tile_id);
	}
	else {
		tile = Bitmap::Create(16, 16, true);
	}

	SetBitmap(tile);

	SetSrcRect({ 0, 0, TILE_SIZE, TILE_SIZE });
	SetOx(8);
	SetOy(16);

	Update();
}

void Sprite_Character::ChipsetUpdated() {
	if (UsesCharset()) {
		return;
	}
	refresh_bitmap = true;
}

Rect Sprite_Character::GetCharacterRect(std::string_view name, int index, const Rect bitmap_rect) {
	Rect rect;
	rect.width = 24 * (TILE_SIZE / 16) * 3;
	rect.height = 32 * (TILE_SIZE / 16) * 4;

	// Allow large 4x2 spriteset of 3x4 sprites
	// when the character name starts with a $ sign.
	// This is not exactly the VX Ace way because
	// VX Ace uses a single 1x1 spriteset of 3x4 sprites.
	if (!name.empty() && name.front() == '$') {
		if (!Player::HasEasyRpgExtensions()) {
			Output::Debug("Ignoring large charset {}. EasyRPG Extension not enabled.", name);
		} else {
			rect.width = bitmap_rect.width * (TILE_SIZE / 16) / 4;
			rect.height = bitmap_rect.height * (TILE_SIZE / 16) / 2;
		}
	}
	rect.x = (index % 4) * rect.width;
	rect.y = (index / 4) * rect.height;
	return rect;
}

void Sprite_Character::OnCharSpriteReady(FileRequestResult*) {
	SetBitmap(Cache::Charset(character_name));
	auto rect = GetCharacterRect(character_name, character_index, GetBitmap()->GetRect());
	chara_width = rect.width / 3;
	chara_height = rect.height / 4;
	SetOx(chara_width / 2);
	SetOy(chara_height);
	SetSpriteRect(rect);

	Update();
}
