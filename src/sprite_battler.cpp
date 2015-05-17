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
#include <boost/bind.hpp>
#include "sprite_battler.h"
#include "async_handler.h"
#include "bitmap.h"
#include "cache.h"
#include "game_enemy.h"
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
	flash_counter(0),
	old_hidden(false) {
	
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

	if (!battler->IsHidden() && old_hidden != battler->IsHidden()) {
		SetOpacity(255);
		SetVisible(true);
		SetAnimationState(AnimationState_Idle);
	}

	old_hidden = battler->IsHidden();

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
		if (Player::IsRPG2k3()) {
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

	if (Player::IsRPG2k3()) {
		if (battler->GetBattleAnimationId() > 0) {
			const RPG::BattlerAnimation& anim = Data::battleranimations[battler->GetBattleAnimationId() - 1];
			const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];
			if (ext.battler_name == sprite_file)
				return;

			sprite_file = ext.battler_name;

			FileRequestAsync* request = AsyncHandler::RequestFile("BattleCharSet", sprite_file);
			request->Bind(boost::bind(&Sprite_Battler::OnBattlercharsetReady, this, _1, ext.battler_index));
			request->Start();
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
		if (sprite_name.empty()) {
			graphic = Bitmap::Create(0, 0);
			SetOx(graphic->GetWidth() / 2);
			SetOy(graphic->GetHeight() / 2);
			SetBitmap(graphic);
		}
		else {
			FileRequestAsync* request = AsyncHandler::RequestFile("Monster", sprite_name);
			request->Bind(&Sprite_Battler::OnMonsterSpriteReady, this);
			request->Start();
		}
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

void Sprite_Battler::OnMonsterSpriteReady(FileRequestResult* result) {
	graphic = Cache::Monster(result->file);

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

void Sprite_Battler::OnBattlercharsetReady(FileRequestResult* result, int battler_index) {
	SetBitmap(Cache::Battlecharset(result->file));
	SetSrcRect(Rect(0, battler_index * 48, 48, 48));
}
