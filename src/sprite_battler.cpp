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
#include "sprite_battler.h"
#include "bitmap.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include "rpg_battleranimation.h"
#include "rpg_battleranimationextension.h"

Sprite_Battler::Sprite_Battler(Game_Battler* battler) :
	battler(battler),
	anim_state(Idle),
	cycle(0),
	sprite_file(""),
	sprite_frame(-1) {
	
	// Not animated
	if (battler->GetBattleAnimationId() == 0) {
		anim_state = 0;
		graphic = Cache::Monster(battler->GetSpriteName());
		SetOx(graphic->GetWidth() / 2);
		SetOy(graphic->GetHeight() / 2);

		/*bool hue_change = battler->GetHue() != 0;
		if (hue_change) {
			BitmapRef new_graphic = Bitmap::Create(graphic->GetWidth(), graphic->GetHeight());
			new_graphic->HueChangeBlit(0, 0, graphic.get(), graphic->GetRect(), battler->GetHue());
			graphic = new_graphic;
		}*/

		SetBitmap(graphic);
	} else { // animated
		SetOx(24);
		SetOy(24);
		SetAnimationState(anim_state);
	}
		
	SetX(battler->GetBattleX());
	SetY(battler->GetBattleY());
	SetZ(battler->GetBattleY()); // Not a typo
	SetVisible(!battler->IsHidden());
}

Sprite_Battler::~Sprite_Battler() {
}

Game_Battler* Sprite_Battler::GetBattler() {
	return battler;
}

void Sprite_Battler::SetBattler(Game_Battler* new_battler) {
	battler = new_battler;
}

void Sprite_Battler::Update() {
	Sprite::Update();
	
	if (Player::engine == Player::EngineRpg2k) {
		static int opacity = 255;
		if (anim_state == Dead && opacity > 0) {
			opacity -= 10;
			SetOpacity(opacity);
		}
	} else if (anim_state > 0) {
		static const int frames[] = {0,1,2,1};
		int frame = frames[cycle / 15];
		if (frame == sprite_frame)
			return;

		const RPG::BattlerAnimation& anim = Data::battleranimations[battler->GetBattleAnimationId() - 1];
		const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];

		SetSrcRect(Rect(frame * 48, ext.battler_index * 48, 48, 48));

		++cycle;
		if (cycle == 60) {
			cycle = 0;
		}
	}
}

void Sprite_Battler::SetAnimationState(int state) {
	anim_state = state;

	if (Player::engine == Player::EngineRpg2k3) {
		const RPG::BattlerAnimation& anim = Data::battleranimations[battler->GetBattleAnimationId() - 1];
		const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];
		if (ext.battler_name == sprite_file)
			return;

		sprite_file = ext.battler_name;
		SetBitmap(Cache::Battlecharset(sprite_file));
	}
}
