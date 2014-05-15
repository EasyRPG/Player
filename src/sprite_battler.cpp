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
	anim_state(AnimationState_Idle),
	cycle(0),
	sprite_file(""),
	sprite_frame(-1),
	fade_out(255),
	flash_counter(0) {
	
	CreateSprite();
}

Sprite_Battler::~Sprite_Battler() {
}

Game_Battler* Sprite_Battler::GetBattler() const {
	return battler;
}

void Sprite_Battler::SetBattler(Game_Battler* new_battler) {
	battler = new_battler;
}

void Sprite_Battler::Update() {
	if (sprite_name != battler->GetSpriteName() ||
		hue != battler->GetHue()) {

		CreateSprite();
	}

	Sprite::Update();

	++cycle;
	
	if (battler->GetBattleAnimationId() <= 0) {
		if (anim_state == AnimationState_Idle) {
			SetOpacity(255);
		}
		else if (anim_state == AnimationState_Dead) {
			if (fade_out > 0) {
				fade_out -= 15;
				SetOpacity(std::max(0, fade_out));
			}
		}
		else if (anim_state == AnimationState_Damage) {
			flash_counter = (flash_counter + 1) % 10;
			SetOpacity(flash_counter > 5 ? 50 : 255);
			if (cycle == 60) {
				SetAnimationState(AnimationState_Idle);
				cycle = 0;
			}
		}
		else {
			if (cycle == 60) {
				SetAnimationState(AnimationState_Idle);
				cycle = 0;
			}
		}
	} else if (anim_state > 0) {
		if (Player::engine == Player::EngineRpg2k3) {
			static const int frames[] = {0,1,2,1};
			int frame = frames[cycle / 10];
			if (frame == sprite_frame)
				return;

			const RPG::BattlerAnimation& anim = Data::battleranimations[battler->GetBattleAnimationId() - 1];
			const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];

			SetSrcRect(Rect(frame * 48, ext.battler_index * 48, 48, 48));

			if (cycle == 40) {
				cycle = 0;

				if (loop_state == LoopState_IdleAnimationAfterFinish) {
					SetAnimationState(AnimationState_Idle);
				}
			}
		}
	}
}

void Sprite_Battler::SetAnimationState(int state, LoopState loop) {
	anim_state = state;

	flash_counter = 0;

	loop_state = loop;

	cycle = 0;

	if (Player::engine == Player::EngineRpg2k3) {
		if (battler->GetBattleAnimationId() > 0) {
			const RPG::BattlerAnimation& anim = Data::battleranimations[battler->GetBattleAnimationId() - 1];
			const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];
			if (ext.battler_name == sprite_file)
				return;

			sprite_file = ext.battler_name;
			SetBitmap(Cache::Battlecharset(sprite_file));
			SetSrcRect(Rect(0, ext.battler_index * 48, 48, 48));
		}
	}
}

bool Sprite_Battler::IsIdling() {
	return anim_state == AnimationState_Idle;
}

void Sprite_Battler::CreateSprite() {
	sprite_name = battler->GetSpriteName();
	hue = battler->GetHue();

	// Not animated -> Monster
	if (battler->GetBattleAnimationId() == 0) {
		graphic = Cache::Monster(sprite_name);
		SetOx(graphic->GetWidth() / 2);
		SetOy(graphic->GetHeight() / 2);

		bool hue_change = hue != 0;
		if (hue_change) {
			BitmapRef new_graphic = Bitmap::Create(graphic->GetWidth(), graphic->GetHeight());
			new_graphic->HueChangeBlit(0, 0, *graphic, graphic->GetRect(), hue);
			graphic = new_graphic;
		}

		SetBitmap(graphic);
	}
	else { // animated
		SetOx(24);
		SetOy(24);
		SetAnimationState(anim_state);
	}

	SetX(battler->GetBattleX());
	SetY(battler->GetBattleY());
	SetZ(battler->GetBattleY()); // Not a typo
	SetVisible(!battler->IsHidden());
}
