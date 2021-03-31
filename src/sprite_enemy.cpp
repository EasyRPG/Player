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
#include "game_screen.h"
#include "sprite_enemy.h"
#include "bitmap.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include <lcf/reader_util.h>
#include "output.h"

Sprite_Enemy::Sprite_Enemy(Game_Enemy* enemy)
	: Sprite_Battler(enemy, enemy->GetTroopMemberId())
{
	CreateSprite();
}

Sprite_Enemy::~Sprite_Enemy() {
}

Game_Enemy* Sprite_Enemy::GetBattler() const {
	return static_cast<Game_Enemy*>(Sprite_Battler::GetBattler());
}

void Sprite_Enemy::CreateSprite() {
	sprite_name = ToString(GetBattler()->GetSpriteName());
	hue = GetBattler()->GetHue();

	SetX(GetBattler()->GetDisplayX());
	SetY(GetBattler()->GetDisplayY());

	// Not animated -> Monster
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
		request_id = request->Bind(&Sprite_Enemy::OnMonsterSpriteReady, this);
		request->Start();
	}
}

void Sprite_Enemy::OnMonsterSpriteReady(FileRequestResult* result) {
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

void Sprite_Enemy::Draw(Bitmap& dst) {

	auto alpha = 255;
	auto zoom = 1.0;

	auto* enemy = static_cast<Game_Enemy*>(GetBattler());

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

	SetOpacity(alpha);
	SetZoomX(zoom);
	SetZoomY(zoom);

	SetTone(Main_Data::game_screen->GetTone());
	SetX(enemy->GetDisplayX());
	SetY(enemy->GetDisplayY());
	SetFlashEffect(enemy->GetFlashColor());
	SetFlipX(enemy->IsDirectionFlipped());

	Sprite_Battler::Draw(dst);
}

void Sprite_Enemy::Refresh() {
	if (sprite_name != GetBattler()->GetSpriteName() || hue != GetBattler()->GetHue()) {
		CreateSprite();
	}
}

void Sprite_Enemy::ResetZ() {
	Sprite_Battler::ResetZ();
}
