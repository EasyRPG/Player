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
#include "game_temp.h"


Sprite_Timer::Sprite_Timer(int which) :
	which(which) {

	if (which != Game_Party::Timer1 &&
		which != Game_Party::Timer2) {
		assert(false && "Invalid timer");
	}

	CreateSprite();
}

Sprite_Timer::~Sprite_Timer() {
}

void Sprite_Timer::Draw() {
	bool timer_visible;
	bool battle;

	Main_Data::game_party->GetTimer(which, timer_visible, battle);

	if (!GetVisible() || !timer_visible) {
		return;
	}

	// In battle but not a battle timer
	if (Game_Temp::battle_running && !battle) {
		return;
	}

	std::string system_name = Game_System::GetSystemName();
	if (system_name.empty()) {
		return;
	}

	BitmapRef system = Cache::System(system_name);

	GetBitmap()->Clear();
	for (int i = 0; i < 5; ++i) {
		if (i == 2) { // :
			if (counter % DEFAULT_FPS >= DEFAULT_FPS / 2) {
				continue;
			}
		}
		GetBitmap()->Blit(i * 8, 0, *system, digits[i], Opacity());
	}

	Sprite::Draw();
}

void Sprite_Timer::Update() {
	bool timer_visible;
	bool battle;

	int time = Main_Data::game_party->GetTimer(which, timer_visible, battle);

	if (time <= 0) {
		SetVisible(false);
		return;
	}
	else {
		++counter;

		if (!GetVisible()) {
			// Wasn't visible until now
			counter = 0;
		}
		SetVisible(true);
	}

	if (!timer_visible) {
		return;
	}

	// In battle but not a battle timer
	if (Game_Temp::battle_running && !battle) {
		return;
	}

	int all_secs = (time + DEFAULT_FPS) / DEFAULT_FPS;

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

	if (Game_Temp::battle_running) {
		SetY(SCREEN_TARGET_HEIGHT / 3 * 2 - 20);
	}
	else if (Game_Message::visible && Game_Message::GetRealPosition() == 0) {
		SetY(SCREEN_TARGET_HEIGHT - 20);
	}
	else {
		SetY(4);
	}

	SetZ(10000);
}

void Sprite_Timer::CreateSprite() {
	for (int i = 0; i < 5; ++i) {
		digits[i] = Rect(0, 32, 8, 16);
	}
	digits[2].x = 32 + 8 * 10; // :

	SetBitmap(Bitmap::Create(40, 16));

	SetVisible(false);

	switch (which) {
		case Game_Party::Timer1:
			SetX(4);
			break;
		case Game_Party::Timer2:
			SetX(SCREEN_TARGET_WIDTH - 8 * 5 - 4);
			break;
		default:
			break;
	}
}
