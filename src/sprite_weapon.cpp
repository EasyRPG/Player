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

Sprite_Weapon::Sprite_Weapon(Game_Actor* actor)
	: Sprite_Battler(actor, actor->GetId())
{
	CreateSprite();
}

Sprite_Weapon::~Sprite_Weapon() {
}

Game_Actor* Sprite_Weapon::GetBattler() const {
	return static_cast<Game_Actor*>(Sprite_Battler::GetBattler());
}


void Sprite_Weapon::Update() {
	auto* battler = GetBattler();

	++cycle;

	// Animations for allies
	// Is a battle charset animation

	int frame = std::min(2, cycle / 10);
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

	const bool flip = battler->IsDirectionFlipped();
	SetFlipX(flip);
}

void Sprite_Weapon::SetWeaponAnimation(int nweapon_animation_id) {
	weapon_animation_id = nweapon_animation_id;
}

void Sprite_Weapon::StartAttack(bool secondary_weapon) {
	cycle = 0;

	attacking = true;

	SetZ(default_z + (secondary_weapon ? 1 : -1));

	auto* battler = GetBattler();

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
	auto* battler = GetBattler();

	SetX(battler->GetDisplayX());
	SetY(battler->GetDisplayY());

	SetOx(32);
	SetOy(32);
	ResetZ();
}

void Sprite_Weapon::OnBattleWeaponReady(FileRequestResult* result, int32_t weapon_index) {
	SetBitmap(Cache::Battleweapon(result->file));
	SetSrcRect(Rect(0, weapon_index * 64, 64, 64));
}

void Sprite_Weapon::Draw(Bitmap& dst) {
	if (!attacking) {
		return;
	}

	auto* battler = GetBattler();

	SetTone(Main_Data::game_screen->GetTone());
	SetX(battler->GetDisplayX());
	SetY(battler->GetDisplayY());
	SetFlashEffect(battler->GetFlashColor());

	Sprite_Battler::Draw(dst);
}

void Sprite_Weapon::ResetZ() {
	constexpr int id_limit = 128;

	const auto& graphic = GetBitmap();
	int y = battler->GetBattlePosition().y;

	int z = battler->GetType();
	z *= SCREEN_TARGET_HEIGHT * 2;
	z += y;
	z *= id_limit;
	z += id_limit - battle_index;
	z += Priority_Battler;

	default_z = z;

	SetZ(z);
}
