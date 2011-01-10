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
#include "cache.h"
#include "game_party.h"
#include "game_actor.h"
#include "window_party.h"

////////////////////////////////////////////////////////////
Window_Party::Window_Party(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::CreateBitmap(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	cycle = 0;
	item_id = 0;

	Refresh();
}

////////////////////////////////////////////////////////////
Window_Party::~Window_Party() {
}

////////////////////////////////////////////////////////////
void Window_Party::Refresh() {
	contents->Clear();

	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	for (size_t i = 0; i < actors.size(); i++) {
		Game_Actor *actor = actors[i];
		const std::string& sprite_name = actor->GetCharacterName();
		int sprite_id = actor->GetCharacterIndex();
		int phase = (cycle / anim_rate) % 3;
		Bitmap *bm = Cache::Charset(sprite_name);
		int width = bm->GetWidth() / 4 / 3;
		int height = bm->GetHeight() / 2 / 4;
		int sx = ((sprite_id % 4) * 3 + phase) * width;
		int sy = ((sprite_id / 4) * 4 + 2) * height;
		Rect src(sx, sy, width, height);
		bool equippable = item_id == 0 || actor->IsEquippable(item_id);
		contents->Blit(i * 32, 0, bm, src, equippable ? 255 : 128);
	}
}

void Window_Party::SetItem(int nitem_id) {
	item_id = nitem_id;
	Refresh();
}

void Window_Party::Update() {
	cycle++;
	if (cycle % anim_rate == 0)
		Refresh();
}

