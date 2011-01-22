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
#include "window_shopparty.h"

////////////////////////////////////////////////////////////
Window_ShopParty::Window_ShopParty(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::CreateBitmap(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	cycle = 0;
	item_id = 0;

	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	for (size_t i = 0; i < actors.size() && i < 4; i++) {
		Game_Actor *actor = actors[i];
		const std::string& sprite_name = actor->GetCharacterName();
		int sprite_id = actor->GetCharacterIndex();
		Bitmap *bm = Cache::Charset(sprite_name);
		int width = bm->GetWidth() / 4 / 3;
		int height = bm->GetHeight() / 2 / 4;
		for (int j = 0; j < 3; j++) {
			int sx = ((sprite_id % 4) * 3 + j) * width;
			int sy = ((sprite_id / 4) * 4 + 2) * height;
			Rect src(sx, sy, width, height);
			for (int k = 0; k < 2; k++) {
				Bitmap *bm2 = Bitmap::CreateBitmap(width, height, true);
				#ifndef USE_ALPHA
				bm2->SetTransparentColor(bm->GetTransparentColor());
				bm2->Clear();
				#endif
				bm2->Blit(0, 0, bm, src, 255);
				if (k == 0)
					bm2->ToneChange(Tone(0, 0, 0, 255));
				bitmaps[i][j][k] = bm2;
			}
		}
	}

	Refresh();
}

////////////////////////////////////////////////////////////
Window_ShopParty::~Window_ShopParty() {
	for (size_t i = 0; i < Game_Party::GetActors().size() && i < 4; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 2; k++)
				delete bitmaps[i][j][k];
}

////////////////////////////////////////////////////////////
void Window_ShopParty::Refresh() {
	contents->Clear();

	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	for (size_t i = 0; i < actors.size() && i < 4; i++) {
		Game_Actor *actor = actors[i];
		int phase = (cycle / anim_rate) % 4;
		if (phase == 3) {
			phase = 1;
		}
		bool equippable = item_id == 0 || actor->IsEquippable(item_id);
		Bitmap *bm = bitmaps[i][phase][equippable ? 1 : 0];
		contents->Blit(i * 32, 0, bm, bm->GetRect(), 255);
	}
}

void Window_ShopParty::SetItemId(int nitem_id) {
	if (nitem_id != item_id) {
		item_id = nitem_id;
		Refresh();
	}
}

void Window_ShopParty::Update() {
	cycle++;
	if (cycle % anim_rate == 0)
		Refresh();
}
