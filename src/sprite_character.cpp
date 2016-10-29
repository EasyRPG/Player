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

Sprite_Character::Sprite_Character(Game_Character* character) :
	character(character),
	tile_id(0),
	character_index(0),
	chara_width(0),
	chara_height(0) {
	Update();
}

void Sprite_Character::Update() {
	Sprite::Update();
	Rect r;
	if (tile_id != character->GetTileId() ||
		character_name != character->GetSpriteName() ||
		character_index != character->GetSpriteIndex()) {
		tile_id = character->GetTileId();
		character_name = character->GetSpriteName();
		character_index = character->GetSpriteIndex();
		if (tile_id > 0) {
			FileRequestAsync* tile_request = AsyncHandler::RequestFile("ChipSet", Game_Map::GetChipsetName());
			request_id = tile_request->Bind(&Sprite_Character::OnTileSpriteReady, this);
			tile_request->Start();
		} else {
			if (character_name.empty()) {
				SetBitmap(BitmapRef());
			} else {
				FileRequestAsync* char_request = AsyncHandler::RequestFile("CharSet", character_name);
				request_id = char_request->Bind(&Sprite_Character::OnCharSpriteReady, this);
				char_request->Start();
			}
		}
	}

	if (tile_id == 0) {
		int row = character->GetSpriteDirection();
		r.Set(character->GetPattern() * chara_width, row * chara_height, chara_width, chara_height);
		SetSrcRect(r);
	}

	if (character->IsFlashPending()) {
		Color col = character->GetFlashColor();
		int dur = character->GetFlashTimeLeft();
		Flash(col, dur);
		// TODO: Gradual decrease of Flash Time Left
		character->SetFlashTimeLeft(0);
	}

	SetVisible(character->GetVisible());
	if (GetVisible()) {
		SetOpacity(character->GetOpacity());
	}

	SetX(character->GetScreenX());
	SetY(character->GetScreenY());
	SetZ(character->GetScreenZ());

	//SetBlendType(character->GetBlendType());
	int bush_split = 4 - character->GetBushDepth();
	SetBushDepth(bush_split > 3 ? 0 : GetHeight() / bush_split);
}

Game_Character* Sprite_Character::GetCharacter() {
	return character;
}
void Sprite_Character::SetCharacter(Game_Character* new_character) {
	character = new_character;
}

void Sprite_Character::OnTileSpriteReady(FileRequestResult*) {
	std::string chipset = Game_Map::GetChipsetName();

	BitmapRef tile;
	if (!chipset.empty()) {
		tile = Cache::Tile(Game_Map::GetChipsetName(), tile_id);
	}
	else {
		tile = Bitmap::Create(16, 16, Color());
	}

	SetBitmap(tile);

	Rect r;
	r.Set(0, 0, TILE_SIZE, TILE_SIZE);
	SetSrcRect(r);
	SetOx(8);
	SetOy(16);

	Update();
}

void Sprite_Character::OnCharSpriteReady(FileRequestResult*) {
	SetBitmap(Cache::Charset(character_name));
	// Allow large 4x2 spriteset of 3x4 sprites
	// when the character name starts with a $ sign.
	// This is not exactly the VX Ace way because
	// VX Ace uses a single 1x1 spriteset of 3x4 sprites.
	if (character_name.size() && character_name.at(0) == '$') {
		chara_width = GetBitmap()->GetWidth() / 4 / 3 * (TILE_SIZE / 16);
		chara_height = GetBitmap()->GetHeight() / 2 / 4 * (TILE_SIZE / 16);
	} else {
		chara_width = 24 * (TILE_SIZE / 16);
		chara_height = 32 * (TILE_SIZE / 16);
	}
	SetOx(chara_width / 2);
	SetOy(chara_height);
	int sx = (character_index % 4) * chara_width * 3;
	int sy = (character_index / 4) * chara_height * 4;
	Rect r;
	r.Set(sx, sy, chara_width * 3, chara_height * 4);
	SetSpriteRect(r);

	Update();
}
