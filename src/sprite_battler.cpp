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
#include "game_enemy.h"
#include "sprite_battler.h"
#include "bitmap.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include "reader_util.h"
#include "output.h"

Sprite_Battler::Sprite_Battler(Game_Battler* battler) :
	battler(battler) {
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
	if (GetVisible() &&
		(sprite_name != battler->GetSpriteName() ||
		hue != battler->GetHue())) {

		CreateSprite();
	}

	if (!battler->IsHidden() && old_hidden != battler->IsHidden()) {
		SetZoomX(1.0);
		SetZoomY(1.0);
		SetOpacity(GetMaxOpacity());
		SetVisible(true);
		DoIdleAnimation();
	}

	old_hidden = battler->IsHidden();

	Sprite::Update();

	++cycle;

	if (battler->GetBattleAnimationId() <= 0) {
		// Animations for monster
		if (anim_state != AnimationState_Dead && anim_state != AnimationState_SelfDestruct) {
			fade_out = GetMaxOpacity();
			fade_out_incr = GetMaxOpacity() * (Player::IsRPG2k() ? 6 : 15) / 255;
			zoom = 1.0;
		}

		if (anim_state == AnimationState_Idle) {
			SetOpacity(GetMaxOpacity());
			idling = true;
		}
		else if (anim_state == AnimationState_Dead) {
			if (fade_out > 0) {
				fade_out -= fade_out_incr;
				SetOpacity(std::max(0, fade_out));
			} else {
				idling = true;
			}
		}
		else if (anim_state == AnimationState_SelfDestruct) {
			if (fade_out > 0) {
				fade_out -= fade_out_incr;
				zoom += 0.07;
				SetOpacity(std::max(0, fade_out));
				SetZoomX(zoom);
				SetZoomY(zoom);
			}
			else {
				idling = true;
			}
		}
		else if (anim_state == AnimationState_Damage) {
			flash_counter = (flash_counter + 1) % 10;
			SetOpacity(flash_counter > 5 ? 50 : GetMaxOpacity());
			if (cycle == 30) {
				DoIdleAnimation();
				cycle = 0;
			}
		}
		else {
			if (cycle == 60) {
				DoIdleAnimation();
				cycle = 0;
			}
		}
	} else if (anim_state > 0) {
		// Animations for allies
		if (Player::IsRPG2k3()) {
			if (animation) {
				// Is a battle animation
				animation->Update();

				if (animation->IsDone()) {
					if (loop_state == LoopState_DefaultAnimationAfterFinish) {
						DoIdleAnimation();
					} else if (loop_state == LoopState_LoopAnimation) {
						animation->SetFrame(0);
					} else if (loop_state == LoopState_WaitAfterFinish) {
						if (animation->GetFrames() > 0) {
							animation->SetFrame(animation->GetFrames() - 1);
						}
						idling = true;
					}
				}

				return;
			}
			// Is a battle charset animation

			static const int frames[] = {0,1,2,1,0};
			int frame = frames[cycle / 10];
			if (frame == sprite_frame)
				return;

			const RPG::BattlerAnimation* anim = ReaderUtil::GetElement(Data::battleranimations, battler->GetBattleAnimationId());
			if (!anim) {
				Output::Warning("Invalid battler animation ID %d", battler->GetBattleAnimationId());
				return;
			}

			const RPG::BattlerAnimationExtension* ext = ReaderUtil::GetElement(anim->base_data, anim_state);
			if (!ext) {
				Output::Warning("Animation %d: Invalid battler anim-extension state %d", anim->ID, anim_state);
				return;
			}

			SetSrcRect(Rect(frame * 48, ext->battler_index * 48, 48, 48));

			if (cycle == 40) {
				switch (loop_state) {
					case LoopState_DefaultAnimationAfterFinish:
						cycle = 0;
						idling = true;
						break;
					case LoopState_WaitAfterFinish:
						--cycle; // incremented to last cycle next update
						idling = true;
						break;
					case LoopState_LoopAnimation:
						cycle = 0;
						break;
					default:
						assert(false && "Bad loop state");
				}
			}

			if (idling) {
				DoIdleAnimation();
			}
		}
	}

	// needed for screen shaking
	SetX(battler->GetDisplayX());
	// needed for flying enemies
	SetY(battler->GetDisplayY());
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
			const RPG::BattlerAnimation* anim = ReaderUtil::GetElement(Data::battleranimations, battler->GetBattleAnimationId());
			if (!anim) {
				Output::Warning("Invalid battler animation ID %d", battler->GetBattleAnimationId());
				return;
			}

			const RPG::BattlerAnimationExtension* ext = ReaderUtil::GetElement(anim->base_data, anim_state);
			if (!ext) {
				Output::Warning("Animation %d: Invalid battler anim-extension state %d", anim->ID, anim_state);
				return;
			}

			sprite_file = ext->battler_name;

			if (ext->animation_type == RPG::BattlerAnimationExtension::AnimType_animation) {
				SetBitmap(BitmapRef());
				RPG::Animation* battle_anim = ReaderUtil::GetElement(Data::animations, ext->animation_id);
				if (!battle_anim) {
					Output::Warning("Invalid battle animation ID %d", ext->animation_id);
					animation.reset();
				} else {
					animation.reset(new BattleAnimationBattlers(*battle_anim, *battler));
					animation->SetZ(GetZ());
				}
			}
			else {
				animation.reset();
				if (!sprite_file.empty()) {
					FileRequestAsync* request = AsyncHandler::RequestFile("BattleCharSet", sprite_file);
					request_id = request->Bind(&Sprite_Battler::OnBattlercharsetReady, this, ext->battler_index);
					request->Start();
				}
			}
		}
	}
}

void Sprite_Battler::SetAnimationLoop(LoopState loop) {
	loop_state = loop;
}

void Sprite_Battler::DetectStateChange() {
	if (idling) {
		DoIdleAnimation();
	}
}

bool Sprite_Battler::IsIdling() {
	return idling;
}

void Sprite_Battler::Flash(int duration) {
	if (animation) {
		animation->Flash(duration);
	} else {
		Sprite::Flash(duration);
	}
}

void Sprite_Battler::Flash(Color color, int duration) {
	if (animation) {
		animation->Flash(color, duration);
	} else {
		Sprite::Flash(color, duration);
	}
}

bool Sprite_Battler::GetVisible() const {
	if (animation) {
		return animation->GetVisible();
	} else {
		return Sprite::GetVisible();
	}
}

void Sprite_Battler::SetVisible(bool nvisible) {
	if (animation) {
		animation->SetVisible(nvisible);
	}
	Sprite::SetVisible(nvisible);
}

int Sprite_Battler::GetWidth() const {
	if (animation) {
		return animation->GetWidth();
	}
	return Sprite::GetWidth();
}

int Sprite_Battler::GetHeight() const {
	if (animation) {
		return animation->GetHeight();
	}
	return Sprite::GetHeight();
}

void Sprite_Battler::CreateSprite() {
	sprite_name = battler->GetSpriteName();
	hue = battler->GetHue();

	SetX(battler->GetDisplayX());
	SetY(battler->GetDisplayY());
	// Battlers at the bottom appear above battlers at the top
	SetZ(Priority_Battler + battler->GetBattleY());

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

void Sprite_Battler::DoIdleAnimation() {
	if (battler->IsDefending()) {
		SetAnimationState(AnimationState_Defending);
		idling = true;
		return;
	}

	const RPG::State* state = battler->GetSignificantState();
	int idling_anim;
	if (battler->GetBattleAnimationId() <= 0) {
		// Monster
		// Only visually different state is Death
		if (state && state->ID == 1) {
			idling_anim = AnimationState_Dead;
		} else {
			idling_anim = AnimationState_Idle;
		}
	} else {
		idling_anim = state ? state->battler_animation_id + 1 : AnimationState_Idle;
	}

	if (idling_anim == 101)
		idling_anim = 7;

	if (idling_anim != anim_state)
		SetAnimationState(idling_anim);

	idling = true;
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

void Sprite_Battler::OnBattlercharsetReady(FileRequestResult* result, int32_t battler_index) {
	SetBitmap(Cache::Battlecharset(result->file));
	SetSrcRect(Rect(0, battler_index * 48, 48, 48));
}

int Sprite_Battler::GetMaxOpacity() const {
	if (battler->GetType() == Game_Battler::Type_Enemy) {
		return static_cast<Game_Enemy*>(battler)->IsTransparent()? 255 - 96 : 255;
	}
	return 255;
}
