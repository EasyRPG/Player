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

Sprite_Character::Sprite_Character(Game_Character* character, CloneType type) :
	character(character),
	tile_id(-1),
	character_index(0),
	chara_width(0),
	chara_height(0) {

	x_shift = ((type & XClone) == XClone);
	y_shift = ((type & YClone) == YClone);

	Update();
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

	if (UsesCharset()) {
		int row = character->GetFacing();
		auto frame = character->GetAnimFrame();
		if (frame >= lcf::rpg::EventPage::Frame_middle2) frame = lcf::rpg::EventPage::Frame_middle;
		SetSrcRect({frame * chara_width, row * chara_height, chara_width, chara_height});
	}

	SetFlashEffect(character->GetFlashColor());

	SetOpacity(character->GetOpacity());
	SetVisible(character->IsVisible());

	SetX(character->GetScreenX(x_shift));
	SetY(character->GetScreenY(y_shift));
	// y_shift because Z is calculated via the screen Y position
	SetZ(character->GetScreenZ(y_shift));

	int bush_split = 4 - character->GetBushDepth();
	SetBushDepth(bush_split > 3 ? 0 : GetHeight() / bush_split);
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

Rect Sprite_Character::GetCharacterRect(StringView name, int index, const Rect bitmap_rect) {
	Rect rect;
	// Allow large 4x2 spriteset of 3x4 sprites
	// when the character name starts with a $ sign.
	// This is not exactly the VX Ace way because
	// VX Ace uses a single 1x1 spriteset of 3x4 sprites.
	if (!name.empty() && name.front() == '$') {
		rect.width = bitmap_rect.width * (TILE_SIZE / 16) / 4;
		rect.height = bitmap_rect.height * (TILE_SIZE / 16) / 2;
	} else {
		rect.width = 24 * (TILE_SIZE / 16) * 3;;
		rect.height = 32 * (TILE_SIZE / 16) * 4;
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
