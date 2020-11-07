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
#include "game_battler.h"
#include "bitmap.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include <lcf/reader_util.h>
#include "output.h"

Sprite_Battler::Sprite_Battler(Game_Battler* battler, int index) :
	battler(battler), battle_index(index) {
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
	if (IsVisible() &&
		(sprite_name != battler->GetSpriteName() ||
		hue != battler->GetHue())) {

		CreateSprite();
	}

	if (!battler->IsHidden() && old_hidden != battler->IsHidden()) {
		DoIdleAnimation();
	}

	old_hidden = battler->IsHidden();

	++cycle;

	if (battler->GetType() == Game_Battler::Type_Ally && anim_state > 0) {
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

			const lcf::rpg::BattlerAnimation* anim = lcf::ReaderUtil::GetElement(lcf::Data::battleranimations, battler->GetBattleAnimationId());
			if (!anim) {
				Output::Warning("Invalid battler animation ID {}", battler->GetBattleAnimationId());
				return;
			}

			const auto* ext = lcf::ReaderUtil::GetElement(anim->poses, anim_state);
			if (!ext) {
				Output::Warning("Animation {}: Invalid battler anim-extension state {}", anim->ID, anim_state);
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

	if (animation) {
		animation->SetVisible(IsVisible());
	}

	const bool flip = battler->IsDirectionFlipped();
	SetFlipX(flip);
	if (animation) {
		SetFlipX(flip);
	}
}

void Sprite_Battler::SetAnimationState(int state, LoopState loop) {
	if (battler->GetType() == Game_Battler::Type_Enemy) {
		return;
	}
	// Default value is 100 (function called with val+1)
	// 100 maps all states to "Bad state" (7)
	if (state == 101) {
		state = 7;
	}

	anim_state = state;

	loop_state = loop;

	cycle = 0;

	idling = false;

	if (Player::IsRPG2k3()) {
		if (battler->GetBattleAnimationId() > 0) {
			const lcf::rpg::BattlerAnimation* anim = lcf::ReaderUtil::GetElement(lcf::Data::battleranimations, battler->GetBattleAnimationId());
			if (!anim) {
				Output::Warning("Invalid battler animation ID {}", battler->GetBattleAnimationId());
				return;
			}

			const auto* ext = lcf::ReaderUtil::GetElement(anim->poses, anim_state);
			if (!ext) {
				Output::Warning("Animation {}: Invalid battler anim-extension state {}", anim->ID, anim_state);
				return;
			}

			StringView sprite_file = ext->battler_name;

			if (ext->animation_type == lcf::rpg::BattlerAnimationPose::AnimType_battle) {
				SetBitmap(BitmapRef());
				lcf::rpg::Animation* battle_anim = lcf::ReaderUtil::GetElement(lcf::Data::animations, ext->battle_animation_id);
				if (!battle_anim) {
					Output::Warning("Invalid battle animation ID {}", ext->battle_animation_id);
					animation.reset();
				} else {
					animation.reset(new BattleAnimationBattle(*battle_anim, { battler }));
					animation->SetZ(GetZ());
				}
			}
			else {
				animation.reset();
				if (!sprite_file.empty()) {
					FileRequestAsync* request = AsyncHandler::RequestFile("BattleCharSet", sprite_file);
					request->SetGraphicFile(true);
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

void Sprite_Battler::ResetZ() {
	static_assert(Game_Battler::Type_Ally < Game_Battler::Type_Enemy, "Game_Battler enums re-ordered! Fix Z order logic here!");

	constexpr int id_limit = 128;

	int y = battler->GetBattlePosition().y;
	if (battler->GetType() == Game_Battler::Type_Enemy && graphic) {
		y += graphic->GetHeight() / 2;
	}

	int z = battler->GetType();
	z *= SCREEN_TARGET_HEIGHT * 2;
	z += y;
	z *= id_limit;
	z += id_limit - battle_index;
	z += Priority_Battler;

	SetZ(z);
}

void Sprite_Battler::CreateSprite() {
	sprite_name = ToString(battler->GetSpriteName());
	hue = battler->GetHue();

	SetX(battler->GetDisplayX());
	SetY(battler->GetDisplayY());

	// Not animated -> Monster
	if (battler->GetBattleAnimationId() == 0) {
		if (sprite_name.empty()) {
			graphic = Bitmap::Create(0, 0);
			SetOx(graphic->GetWidth() / 2);
			SetOy(graphic->GetHeight() / 2);
			SetBitmap(graphic);
			ResetZ();
		}
		else {
			FileRequestAsync* request = AsyncHandler::RequestFile("Monster", sprite_name);
			request->SetGraphicFile(true);
			request_id = request->Bind(&Sprite_Battler::OnMonsterSpriteReady, this);
			request->Start();
		}
	}
	else { // animated
		SetOx(24);
		SetOy(24);
		ResetZ();
		SetAnimationState(anim_state);
		idling = true;
	}
}

void Sprite_Battler::DoIdleAnimation() {
	if (battler->IsDefending()) {
		SetAnimationState(AnimationState_Defending);
		idling = true;
		return;
	}

	const lcf::rpg::State* state = battler->GetSignificantState();
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

	ResetZ();

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

void Sprite_Battler::Draw(Bitmap& dst) {

	auto alpha = 255;
	auto zoom = 1.0;

	if (battler->GetType() == Game_Battler::Type_Enemy) {
		auto* enemy = static_cast<Game_Enemy*>(battler);

		const auto bt = enemy->GetBlinkTimer();
		const auto dt = enemy->GetDeathTimer();
		const auto et = enemy->GetExplodeTimer();

		if (!enemy->Exists() && dt == 0 && et == 0) {
			return;
		}

		if (bt % 10 >= 5) {
			return;
		}

		if (dt > 0) {
			alpha = 7 * dt;
		} else if (et > 0) {
			alpha = 12 * et;
			zoom = static_cast<double>(20 - et) / 20.0 + 1.0;
		}

		if (enemy->IsTransparent()) {
			alpha = 160 * alpha / 255;
		}
	} else {
		if (battler->IsHidden()) {
			return;
		}
	}

	SetOpacity(alpha);
	SetZoomX(zoom);
	SetZoomY(zoom);
	SetX(battler->GetDisplayX());
	SetY(battler->GetDisplayY());
	SetFlashEffect(battler->GetFlashColor());

	Sprite::Draw(dst);
}
