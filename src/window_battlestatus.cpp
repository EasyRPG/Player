/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "bitmap.h"
#include "surface.h"
#include "cache.h"
#include "input.h"
#include "game_party.h"
#include "game_actor.h"
#include "game_system.h"
#include "window_battlestatus.h"

////////////////////////////////////////////////////////////
Window_BattleStatus::Window_BattleStatus() :
	Window_Base(0, 172, 244, 68),
	actors(Game_Party::GetActors()) {

	SetBorderX(4);
	SetBorderY(4);

	SetContents(Surface::CreateSurface(width - 8, height - 8));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	index = -1;
	std::fill(gauges, gauges + 4, 0);

	Refresh();
}

////////////////////////////////////////////////////////////
Window_BattleStatus::~Window_BattleStatus() {
}

////////////////////////////////////////////////////////////
void Window_BattleStatus::Refresh() {
	contents->Clear();

	for (size_t i = 0; i < actors.size() && i < 4; i++) {
		int y = i * 15;
		Game_Actor* actor = actors[i];
		DrawActorName(actor, 4, y);
		DrawActorState(actor, 80, y);
		DrawActorHp(actor, 136, y, true);
		DrawGauge(actor, i, 192, y);
		DrawActorSp(actor, 202, y, false);
	}
}

////////////////////////////////////////////////////////////
void Window_BattleStatus::RefreshGauge(int i) {
	int y = i * 15;
	contents->ClearRect(Rect(192, y, 57, 16));
	Game_Actor* actor = actors[i];
	DrawGauge(actor, i, 192, y);
	DrawActorSp(actor, 202, y, false);
}

////////////////////////////////////////////////////////////
void Window_BattleStatus::DrawGauge(Game_Actor* actor, int index, int cx, int cy) {
	Bitmap* system2 = Cache::System2(Data::system.system2_name);

	bool full = gauges[index] == gauge_full;
	int gauge = gauges[index] * 25 / gauge_full;
	int speed = 2; // FIXME: how to determine?
	int gauge_y = 32 + speed * 16;
	Rect gauge_left(0, gauge_y, 16, 16);
	Rect gauge_center(16, gauge_y, 16, 16);
	Rect gauge_right(32, gauge_y, 16, 16);
	Rect gauge_bar(full ? 64 : 48, gauge_y, 16, 16);
	Rect dst_rect(cx+16, cy, 25, 16);
	Rect bar_rect(cx+16, cy, gauge, 16);

	contents->Blit(cx+0, cy, system2, gauge_left, 255);
	contents->StretchBlit(dst_rect, system2, gauge_center, 255);
	contents->Blit(cx+16+25, cy, system2, gauge_right, 255);
	contents->StretchBlit(bar_rect, system2, gauge_bar, 255);
}

////////////////////////////////////////////////////////////
void Window_BattleStatus::SetActiveCharacter(int _index) {
	index = _index;
	Refresh();
}

////////////////////////////////////////////////////////////
int Window_BattleStatus::GetActiveCharacter() {
	return index;
}

////////////////////////////////////////////////////////////
void Window_BattleStatus::SetTimeGauge(int _index, int value, int limit) {
	gauges[_index] = value * gauge_full / limit;
	RefreshGauge(_index);

	int num_actors = actors.size();
	int old_index = index < 0 ? 0 : index;
	index = -1;
	for (int i = 0; i < num_actors; i++) {
		int new_index = (old_index + i) % num_actors;
		if (gauges[new_index] == gauge_full) {
			index = new_index;
			break;
		}
	}

	if (index != old_index)
		UpdateCursorRect();
}


////////////////////////////////////////////////////////////
void Window_BattleStatus::Update() {
	Window_Base::Update();
	if (active && index >= 0) {
		int num_actors = actors.size();

		if (Input::IsRepeated(Input::DOWN)) {
			Game_System::SePlay(Data::system.cursor_se);
			for (int i = 1; i < num_actors; i++) {
				int new_index = (index + i) % num_actors;
				if (gauges[new_index] == gauge_full) {
					index = new_index;
					break;
				}
			}
		}
		if (Input::IsRepeated(Input::UP)) {
			Game_System::SePlay(Data::system.cursor_se);
			for (int i = num_actors - 1; i > 0; i--) {
				int new_index = (index + i) % num_actors;
				if (gauges[new_index] == gauge_full) {
					index = new_index;
					break;
				}
			}
		}
	}

	UpdateCursorRect();
}

////////////////////////////////////////////////////////////
void Window_BattleStatus::UpdateCursorRect() {
	if (index < 0)
		SetCursorRect(Rect());
	else
		SetCursorRect(Rect(0, index * 15, contents->GetWidth(), 16));
}

