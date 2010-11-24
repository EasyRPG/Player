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
#include "sprite_character.h"
#include "cache.h"
#include "game_map.h"

////////////////////////////////////////////////////////////
Sprite_Character::Sprite_Character(Game_Character* character) :
	character(character),
	tile_id(0),
	character_index(0),
	chara_width(0),
	chara_height(0) {
	Update();
}

////////////////////////////////////////////////////////////
Sprite_Character::~Sprite_Character() {
}

////////////////////////////////////////////////////////////
void Sprite_Character::Update() {
	Sprite::Update();
	Rect r;
	if (tile_id != character->GetTileId() ||
		character_name != character->GetCharacterName() ||
		character_index != character->GetCharacterIndex()) {
		tile_id = character->GetTileId();
		character_name = character->GetCharacterName();
		character_index = character->GetCharacterIndex();
		if (tile_id > 0) {
			SetBitmap(Cache::Tile(Game_Map::GetChipsetName(), tile_id));
			r.Set(0, 0, 16, 16);
			SetSrcRect(r);
			SetOx(8);
			SetOy(16);
		} else {
			SetBitmap(Cache::Charset(character_name));
			chara_width = GetBitmap()->GetWidth() / 4 / 3;
			chara_height = GetBitmap()->GetHeight() / 2 / 4;
			SetOx(chara_width / 2);
			SetOy(chara_height);
			int sx = (character_index % 4) * chara_width * 3;
			int sy = (character_index / 4) * chara_height * 4;
			r.Set(sx, sy, chara_width * 3, chara_height * 4);
			SetSrcRect(r);
		}
	}
	//SetVisible(character->GetVisible());
	if (tile_id == 0) {
		int row;
		switch (character->GetDirection()) {
			case 2: row = 2; break;
			case 4: row = 3; break;
			case 6: row = 1; break;
			case 8: row = 0; break;
		}
		r.Set(character->GetPattern() * chara_width, row * chara_height, chara_width, chara_height);
		SetSpriteRect(r);
	}
	SetX(character->GetScreenX());
	SetY(character->GetScreenY());
	SetZ(character->GetScreenZ(chara_height));
	//SetOpacity(character->GetOpacity());
	//SetBlendType(character->GetBlendType());
	//SetBushDepth(character->GetBushDepth());
	/*if (character->GetAnimationId() != 0) {
		RPG::Animation animation = Data::animations[character->GetAnimationId()]
		this->Animation(animation, true);
		character->SetAnimationId(0);
	}*/
}

////////////////////////////////////////////////////////////
Game_Character* Sprite_Character::GetCharacter() {
	return character;
}
void Sprite_Character::SetCharacter(Game_Character* new_character) {
	character = new_character;
}
