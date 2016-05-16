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
#include "battle_animation.h"
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
	flash_counter(0),
	old_hidden(false),
	idling(true) {

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
		idling = true;
	}

	old_hidden = battler->IsHidden();

	Sprite::Update();

	++cycle;

	if (battler->GetBattleAnimationId() <= 0) {
		// Animations for monster
		if (anim_state == AnimationState_Idle) {
			SetOpacity(255);
			idling = true;
		}
		else if (anim_state == AnimationState_Dead) {
			if (fade_out > 0) {
				fade_out -= 15;
				SetOpacity(std::max(0, fade_out));
			} else {
				idling = true;
			}
		}
		else if (anim_state == AnimationState_Damage) {
			flash_counter = (flash_counter + 1) % 10;
			SetOpacity(flash_counter > 5 ? 50 : 255);
			if (cycle == 60) {
				SetAnimationState(AnimationState_Idle);
				idling = true;
				cycle = 0;
			}
		}
		else {
			if (cycle == 60) {
				SetAnimationState(AnimationState_Idle);
				idling = true;
				cycle = 0;
			}
		}
	} else if (anim_state > 0) {
		// Animations for allies
		if (Player::IsRPG2k3()) {
			if (animation) {
				animation->Update();

				if (animation->IsDone()) {
					if (loop_state == LoopState_DefaultAnimationAfterFinish) {
						const RPG::State* state = battler->GetSignificantState();
						if (state) {
							SetAnimationState(state->battler_animation_id + 1);
						} else {
							SetAnimationState(AnimationState_Idle);
						}
						idling = true;
					} else if (loop_state == LoopState_LoopAnimation) {
						animation->SetFrame(0);
					} else if (loop_state == LoopState_WaitAfterFinish) {
						idling = true;
					}
				}

				return;
			}

			static const int frames[] = {0,1,2,1,0};
			int frame = frames[cycle / 10];
			if (frame == sprite_frame)
				return;

			const RPG::BattlerAnimation& anim = Data::battleranimations[battler->GetBattleAnimationId() - 1];
			const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];

			SetSrcRect(Rect(frame * 48, ext.battler_index * 48, 48, 48));

			if (cycle == 40) {
				switch (loop_state) {
					case LoopState_DefaultAnimationAfterFinish:
						cycle = 0;
						// fallthrough
					case LoopState_WaitAfterFinish:
						idling = true;
						break;
					case LoopState_LoopAnimation:
						cycle = 0;
						break;
				}
			}

			if (idling) {
				const RPG::State* state = battler->GetSignificantState();
				int idling_anim = state ? state->battler_animation_id + 1 : AnimationState_Idle;
				if (idling_anim == 101)
					idling_anim = 7;

				if (idling_anim != anim_state)
					SetAnimationState(idling_anim);
				idling = true;
			}
		}
	}
}

void Sprite_Battler::SetAnimationState(int state, LoopState loop) {
	// Default value is 100 (function called with val+1)
	// 100 maps all states to "Bad state" (7)
	if (state == 101) {
		state = 7;
	}

	anim_state = state;

	flash_counter = 0;

	loop_state = loop;

	cycle = 0;

	idling = false;

	if (Player::IsRPG2k3()) {
		if (battler->GetBattleAnimationId() > 0) {
			const RPG::BattlerAnimation& anim = Data::battleranimations[battler->GetBattleAnimationId() - 1];
			const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];

			sprite_file = ext.battler_name;

			if (ext.animation_type == RPG::BattlerAnimationExtension::AnimType_animation) {
				SetBitmap(BitmapRef());
				animation.reset(new BattleAnimationBattlers(Data::animations[ext.animation_id - 1], *battler));
				animation->SetZ(GetZ());
			}
			else {
				animation.reset();
				if (!sprite_file.empty()) {
					FileRequestAsync* request = AsyncHandler::RequestFile("BattleCharSet", sprite_file);
					request_id = request->Bind(&Sprite_Battler::OnBattlercharsetReady, this, ext.battler_index);
					request->Start();
				}
			}
		}
	}
}

void Sprite_Battler::SetAnimationLoop(LoopState loop) {
	loop_state = loop;
}

void Sprite_Battler::DetectDeath() {
	if (battler->IsDead() && anim_state != AnimationState_Dead) {
		SetAnimationState(AnimationState_Dead);
	}
}

bool Sprite_Battler::IsIdling() {
	return idling;
}

void Sprite_Battler::Flash(int duration) {
	if (animation) {
		animation->GetSprite()->Flash(duration);
	} else {
		Sprite::Flash(duration);
	}
}

void Sprite_Battler::Flash(Color color, int duration) {
	if (animation) {
		animation->GetSprite()->Flash(color, duration);
	} else {
		Sprite::Flash(color, duration);
	}
}

bool Sprite_Battler::GetVisible() const {
	if (animation) {
		return animation->GetSprite()->GetVisible();
	} else {
		return Sprite::GetVisible();
	}
}

void Sprite_Battler::SetVisible(bool nvisible) {
	if (animation) {
		animation->GetSprite()->SetVisible(nvisible);
	}
	Sprite::SetVisible(nvisible);
}

int Sprite_Battler::GetWidth() const {
	if (animation) {
		return animation->GetSprite()->GetWidth();
	}
	return Sprite::GetWidth();
}

int Sprite_Battler::GetHeight() const {
	if (animation) {
		return animation->GetSprite()->GetHeight();
	}
	return Sprite::GetHeight();
}

void Sprite_Battler::CreateSprite() {
	sprite_name = battler->GetSpriteName();
	hue = battler->GetHue();

	SetX(battler->GetBattleX());
	SetY(battler->GetBattleY());
	SetZ(battler->GetBattleY()); // Not a typo

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
			request_id = request->Bind(&Sprite_Battler::OnMonsterSpriteReady, this);
			request->Start();
		}
	}
	else { // animated
		SetOx(24);
		SetOy(24);
		SetAnimationState(anim_state);
		idling = true;
	}

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
