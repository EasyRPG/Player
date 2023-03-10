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
#include "sprite_timer.h"
#include "cache.h"
#include "bitmap.h"
#include "game_message.h"
#include "game_party.h"
#include "game_system.h"
#include "game_battle.h"
#include <player.h>

Sprite_Timer::Sprite_Timer(int which) :
	which(which)
{
	if (which != Game_Party::Timer1 &&
		which != Game_Party::Timer2) {
		assert(false && "Invalid timer");
	}

	for (auto& d: digits) {
		d = Rect(0, 32, 8, 16);
	}
	digits[2].x = 32 + 8 * 10; // :

	SetBitmap(Bitmap::Create(40, 16));

	SetVisible(false);

	switch (which) {
		case Game_Party::Timer1:
			SetX(Player::menu_offset_x + 4);
			break;
		case Game_Party::Timer2:
			SetX(Player::screen_width - 8 * 5 - 4 - Player::menu_offset_x);
			break;
		default:
			break;
	}

	SetZ(Priority_Timer);
	SetVisible(true);
}

Sprite_Timer::~Sprite_Timer() {
}

void Sprite_Timer::Draw(Bitmap& dst) {
	if (!Main_Data::game_party->GetTimerVisible(which, Game_Battle::IsBattleRunning())) {
		return;
	}

	// RPG_RT never displays timers if there is no system graphic.
	BitmapRef system = Cache::System();
	if (!system) {
		return;
	}

	const int all_secs = Main_Data::game_party->GetTimerSeconds(which);

	int mins = all_secs / 60;
	int secs = all_secs % 60;

	int mins_1 = mins % 10;
	int mins_10 = mins / 10;

	int secs_1 = secs % 10;
	int secs_10 = secs / 10;

	digits[0].x = 32 + 8 * mins_10;
	digits[1].x = 32 + 8 * mins_1;
	digits[3].x = 32 + 8 * secs_10;
	digits[4].x = 32 + 8 * secs_1;

	if (Game_Battle::IsBattleRunning()) {
		SetY((Player::screen_height / 3 * 2) - 20);
	}
	else if (Game_Message::IsMessageActive() && Game_Message::GetRealPosition() == 0) {
		SetY(Player::screen_height - 20 - Player::menu_offset_y);
	}
	else {
		SetY(Player::menu_offset_y + 4);
	}

	GetBitmap()->Clear();
	for (int i = 0; i < 5; ++i) {
		if (i == 2) { // :
			int frames =  Main_Data::game_party->GetTimerFrames(which);
			if (frames % DEFAULT_FPS < DEFAULT_FPS / 2) {
				continue;
			}
		}
		GetBitmap()->Blit(i * 8, 0, *system, digits[i], Opacity());
	}

	Sprite::Draw(dst);
}

