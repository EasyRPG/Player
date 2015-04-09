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
	chara_width(24*(TILE_SIZE/16)),
	chara_height(32*(TILE_SIZE/16)) {
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
			BitmapRef tile = Cache::Tile(Game_Map::GetChipsetName(), tile_id);
			SetBitmap(tile);
			r.Set(0, 0, TILE_SIZE, TILE_SIZE);
			SetSrcRect(r);
			SetOx(8);
			SetOy(16);
		} else {
			if (character_name.empty()) {
				SetBitmap(BitmapRef());
			} else {
				SetBitmap(Cache::Charset(character_name));
				SetOx(chara_width / 2);
				SetOy(chara_height);
				int sx = (character_index % 4) * chara_width * 3;
				int sy = (character_index / 4) * chara_height * 4;
				r.Set(sx, sy, chara_width * 3, chara_height * 4);
				SetSpriteRect(r);
			}
		}
	}

	if (tile_id == 0) {
		int row = (character->IsSpinning() ? character->GetPrelockDirection() : character->GetDirection());
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
