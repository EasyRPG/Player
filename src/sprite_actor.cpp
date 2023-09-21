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
#include "sprite_actor.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_screen.h"
#include "bitmap.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include <lcf/reader_util.h>
#include "output.h"
#include "feature.h"
#include "game_battle.h"

Sprite_Actor::Sprite_Actor(Game_Actor* actor)
	: Sprite_Battler(actor, actor->GetId())
{
	CreateSprite();
	auto condition = Game_Battle::GetBattleCondition();
	if ((condition == lcf::rpg::System::BattleCondition_none || condition == lcf::rpg::System::BattleCondition_initiative) && Feature::HasFixedActorFacingDirection()) {
		fixed_facing = static_cast<FixedFacing>(lcf::Data::battlecommands.easyrpg_fixed_actor_facing_direction);
	}
}

Sprite_Actor::~Sprite_Actor() {
}

Game_Actor* Sprite_Actor::GetBattler() const {
	return static_cast<Game_Actor*>(Sprite_Battler::GetBattler());
}

void Sprite_Actor::Update() {
	auto* battler = GetBattler();

	if (!battler->IsHidden() && old_hidden != battler->IsHidden()) {
		DoIdleAnimation();
	}

	old_hidden = battler->IsHidden();

	++cycle;

	if (anim_state > 0) {
		// Animations for allies
		if (Player::IsRPG2k3()) {
			UpdatePosition();

			if (animation) {
				// Is a battle animation
				animation->SetInvert(battler->IsSpriteDirectionFlipped());
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
			int frame = (battler->IsDefending() ? 0 : (normal_attacking ? std::min(2, cycle / 10) : frames[cycle / 10]));

			if (battler->IsDirectionFlipped()) {
				frame = 2 - frame;
			}

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

			if (cycle == ((idling || normal_attacking || anim_state == AnimationState_WalkingLeft || anim_state == AnimationState_WalkingRight || anim_state == AnimationState_Victory) ? 40 : 30)) {
				switch (loop_state) {
					case LoopState_DefaultAnimationAfterFinish:
						DoIdleAnimation();
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
		}
	}

	if (animation) {
		animation->SetVisible(IsVisible());
	}

	SetFlipX(battler->IsDirectionFlipped());
}

void Sprite_Actor::SetAnimationState(int state, LoopState loop, int animation_id) {
	// Default value is 100 (function called with val+1)
	// 100 maps all states to "Bad state" (7)
	if (state == 101) {
		state = 7;
	}

	anim_state = state;

	loop_state = loop;

	cycle = 0;

	idling = false;

	auto* battler = GetBattler();

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
			do_not_draw = false;
			SetBitmap(BitmapRef());
			if (animation_id == 0) {
				animation_id = ext->battle_animation_id;
			}
			lcf::rpg::Animation* battle_anim = lcf::ReaderUtil::GetElement(lcf::Data::animations, animation_id);
			if (!battle_anim) {
				Output::Warning("Invalid battle animation ID {}", animation_id);
				animation.reset();
			} else {
				animation.reset(new BattleAnimationBattler(*battle_anim, { battler }));
				animation->SetZ(GetZ());
			}
			animation->SetInvert(battler->IsDirectionFlipped());
		}
		else {
			do_not_draw = sprite_file.empty();
			animation.reset();
			if (!sprite_file.empty()) {
				FileRequestAsync* request = AsyncHandler::RequestFile("BattleCharSet", sprite_file);
				request->SetGraphicFile(true);
				request_id = request->Bind(&Sprite_Actor::OnBattlercharsetReady, this, ext->battler_index);
				request->Start();
			}
		}
	}
}

void Sprite_Actor::SetAnimationLoop(LoopState loop) {
	loop_state = loop;
}

void Sprite_Actor::DetectStateChange() {
	if (idling) {
		DoIdleAnimation();
	}
}

bool Sprite_Actor::IsIdling() {
	return idling;
}

int Sprite_Actor::GetWidth() const {
	if (animation) {
		return animation->GetAnimationCellWidth() / 2;
	}
	return Sprite::GetWidth();
}

int Sprite_Actor::GetHeight() const {
	if (animation) {
		return animation->GetAnimationCellHeight() / 2;
	}
	return Sprite::GetHeight();
}

void Sprite_Actor::CreateSprite() {
	auto* battler = GetBattler();

	images = {{battler->GetDisplayX(), battler->GetDisplayY()}};

	SetX(images.back().x);
	SetY(images.back().y);

	// Not animated -> Monster
	SetOx(24);
	SetOy(24);
	ResetZ();
	SetAnimationState(anim_state);
	idling = true;
}

void Sprite_Actor::DoIdleAnimation() {
	auto* battler = GetBattler();
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

	if (idling_anim != anim_state || loop_state == LoopState_DefaultAnimationAfterFinish)
		SetAnimationState(idling_anim, idling_anim == AnimationState_Dead ? LoopState_WaitAfterFinish : LoopState_LoopAnimation);

	idling = true;
}

void Sprite_Actor::OnBattlercharsetReady(FileRequestResult* result, int32_t battler_index) {
	SetBitmap(Cache::Battlecharset(result->file));
	SetSrcRect(Rect(0, battler_index * 48, 48, 48));
}

void Sprite_Actor::Draw(Bitmap& dst) {
	auto* battler = GetBattler();
	// "do_not_draw" is set to true if the CBA battler name is empty, this
	// makes the sprite not being drawn. This fixes issue #1708.
	if (battler->IsHidden() || do_not_draw) {
		return;
	}

	SetTone(Main_Data::game_screen->GetTone());
	SetFlashEffect(battler->GetFlashColor());

	int steps = static_cast<int>(256 / images.size());
	int opacity = steps;
	for (auto it = images.crbegin(); it != images.crend(); ++it) {
		Sprite_Battler::SetFixedFlipX();
		Sprite_Battler::SetX(it->x);
		Sprite_Battler::SetY(it->y);
		Sprite_Battler::SetOpacity(std::min(opacity, 255));
		Sprite_Battler::Draw(dst);
		opacity += steps;
	}
}

void Sprite_Actor::UpdatePosition() {
	assert(!images.empty());
	images.pop_back();
	images.insert(images.begin(), {battler->GetDisplayX(), battler->GetDisplayY()});

	if (afterimage_fade >= 0) {
		++afterimage_fade;
		if (static_cast<size_t>(afterimage_fade) >= images.size()) {
			// CBA finished, remove afterimages
			images.resize(1);
			afterimage_fade = -1;
		}
	}
}

void Sprite_Actor::ResetZ() {
	Sprite_Battler::ResetZ();
	if (animation) {
		animation->SetZ(GetZ());
	}
}

void Sprite_Actor::SetNormalAttacking(bool nnormal_attacking) {
	normal_attacking = nnormal_attacking;
}

void Sprite_Actor::SetAfterimageAmount(unsigned amount) {
	images.resize(1 + amount);
	std::fill(images.begin() + 1, images.end(), images.front());
}

void Sprite_Actor::DoAfterimageFade() {
	if (images.size() > 1) {
		afterimage_fade = 0;
	}
}
