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
#include "sprite_weapon.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_screen.h"
#include "bitmap.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include <lcf/reader_util.h>
#include "output.h"

Sprite_Weapon::Sprite_Weapon(Game_Actor* actor) : Sprite() {
	battler = actor;
	CreateSprite();
}

Sprite_Weapon::~Sprite_Weapon() {
}


void Sprite_Weapon::Update() {
	++cycle;

	// Animations for allies
	// Is a battle charset animation

	int frame = std::min(2, cycle / 10);

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

	const auto* battler_animation_weapon = lcf::ReaderUtil::GetElement(anim->weapons, weapon_animation_id);
	if (!battler_animation_weapon) {
		return;
	}

	SetSrcRect(Rect(frame * 64, battler_animation_weapon->weapon_index * 64, 64, 64));

	SetFlipX(battler->IsDirectionFlipped());
}

void Sprite_Weapon::SetWeaponAnimation(int nweapon_animation_id) {
	weapon_animation_id = nweapon_animation_id;
}

void Sprite_Weapon::SetRanged(bool nranged) {
	ranged = nranged;
}

void Sprite_Weapon::StartAttack(bool secondary_weapon) {
	if ((secondary_weapon && !battler->IsDirectionFlipped()) || (!secondary_weapon && battler->IsDirectionFlipped())) {
		SetZ(battler->GetBattleSprite()->GetZ() + 1);
	} else {
		SetZ(battler->GetBattleSprite()->GetZ() - 1);
	}

	cycle = 0;

	attacking = true;

	if (battler->GetBattleAnimationId() > 0) {
		const lcf::rpg::BattlerAnimation* anim = lcf::ReaderUtil::GetElement(lcf::Data::battleranimations, battler->GetBattleAnimationId());
		if (!anim) {
			Output::Warning("Invalid battler animation ID {}", battler->GetBattleAnimationId());
			return;
		}

		const auto* battler_animation_weapon = lcf::ReaderUtil::GetElement(anim->weapons, weapon_animation_id);
		if (!battler_animation_weapon) {
			Output::Warning("Invalid weapon animation ID {}", weapon_animation_id);
			return;
		}

		StringView sprite_file = battler_animation_weapon->weapon_name;

		if (!sprite_file.empty()) {
			FileRequestAsync* request = AsyncHandler::RequestFile("BattleWeapon", sprite_file);
			request->SetGraphicFile(true);
			request_id = request->Bind(&Sprite_Weapon::OnBattleWeaponReady, this, battler_animation_weapon->weapon_index);
			request->Start();
		}
	}
}

void Sprite_Weapon::StopAttack() {
	attacking = false;
}

void Sprite_Weapon::CreateSprite() {
	SetX(battler->GetDisplayX());
	SetY(battler->GetDisplayY());

	SetOx(32);
	SetOy(32);
}

void Sprite_Weapon::OnBattleWeaponReady(FileRequestResult* result, int32_t weapon_index) {
	SetBitmap(Cache::Battleweapon(result->file));
	SetFlipX(battler->IsDirectionFlipped());
	SetSrcRect(Rect(0, weapon_index * 64, 64, 64));
}

void Sprite_Weapon::Draw(Bitmap& dst) {
	if (!attacking) {
		return;
	}

	SetTone(Main_Data::game_screen->GetTone());
	if (!ranged) {
		SetX(battler->GetDisplayX());
		SetY(battler->GetDisplayY());
	}
	SetFlashEffect(battler->GetFlashColor());

	Sprite::Draw(dst);
}
