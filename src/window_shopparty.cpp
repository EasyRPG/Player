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
#include "bitmap.h"
#include "cache.h"
#include "game_party.h"
#include "game_actor.h"
#include "window_shopparty.h"

Window_ShopParty::Window_ShopParty(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	cycle = 0;
	item_id = 0;

	const std::vector<Game_Actor*>& actors = Main_Data::game_party->GetActors();
	for (size_t i = 0; i < actors.size() && i < 4; i++) {
		const std::string& sprite_name = actors[i]->GetSpriteName();
		FileRequestAsync* request = AsyncHandler::RequestFile("CharSet", sprite_name);
		request_ids.push_back(request->Bind(&Window_ShopParty::OnCharsetSpriteReady, this, (int)i));
		request->Start();
	}

	Refresh();
}

void Window_ShopParty::Refresh() {
	contents->Clear();

	BitmapRef system = Cache::System();

	if (item_id <= 0 || item_id > static_cast<int>(Data::items.size()))
		return;

	const std::vector<Game_Actor*>& actors = Main_Data::game_party->GetActors();
	for (size_t i = 0; i < actors.size() && i < 4; i++) {
		Game_Actor *actor = actors[i];
		int phase = (cycle / anim_rate) % 4;
		if (phase == 3) {
			phase = 1;
		}
		bool equippable = actor->IsEquippable(item_id);
		BitmapRef bm = bitmaps[i][equippable ? phase : 1][equippable ? 1 : 0];

		if (bm) {
			contents->Blit(i * 32, 0, *bm, bm->GetRect(), 255);
		}

		if (equippable) {
			//check if item is equipped by each member
			bool is_equipped = false;
			for (int j = 1; j <= 5; ++j) {
				const RPG::Item* item = actor->GetEquipment(j);
				if (item) {
					is_equipped |= (item->ID == item_id);
				}
			}
			if (is_equipped)
				contents->Blit(i * 32 + 20, 24, *system, Rect(128 + 8 * phase, 24, 8, 8), 255);
			else {

				RPG::Item* new_item = &Data::items[item_id - 1];
				int item_type =  new_item->type;
				RPG::Item* current_item = NULL;

				switch (item_type) {

				//get the current equipped item
				case RPG::Item::Type_weapon:
					if (actor->GetWeaponId() > 0)
						current_item = &Data::items[actor->GetWeaponId() - 1];
					else
						current_item = &Data::items[0];
					break;
				case RPG::Item::Type_helmet:
					if (actor->GetHelmetId() > 0)
						current_item = &Data::items[actor->GetHelmetId() - 1];
					else
						current_item = &Data::items[0];
					break;
				case RPG::Item::Type_shield:
					if (actor->GetShieldId() > 0)
						current_item = &Data::items[actor->GetShieldId() - 1];
					else
						current_item = &Data::items[0];
					break;
				case RPG::Item::Type_armor:
					if (actor->GetArmorId() > 0)
						current_item = &Data::items[actor->GetArmorId() - 1];
					else
						current_item = &Data::items[0];
					break;
				case RPG::Item::Type_accessory:
					if (actor->GetAccessoryId() > 0)
						current_item = &Data::items[actor->GetAccessoryId() -1];
					else
						current_item = &Data::items[0];
					break;
				}

				if (current_item != NULL) {
					int diff_atk = new_item->atk_points1 - current_item->atk_points1;
					int diff_def = new_item->def_points1 - current_item->def_points1;
					int diff_spi = new_item->spi_points1 - current_item->spi_points1;
					int diff_agi = new_item->agi_points1 - current_item->agi_points1;
					if (diff_atk > 0 || diff_def > 0 || diff_spi > 0 || diff_agi > 0)
						contents->Blit(i * 32 + 20, 24, *system, Rect(128 + 8 * phase, 0, 8, 8), 255);
					else if (diff_atk < 0 || diff_def < 0 || diff_spi < 0 || diff_agi < 0)
						contents->Blit(i * 32 + 20, 24, *system, Rect(128 + 8 * phase, 16, 8, 8), 255);
					else
						contents->Blit(i * 32 + 20, 24, *system, Rect(128 + 8 * phase, 8, 8, 8), 255);
				}
			}
		}
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

void Window_ShopParty::OnCharsetSpriteReady(FileRequestResult* /* result */, int party_index) {
	Game_Actor *actor = Main_Data::game_party->GetActors()[party_index];
	const std::string& sprite_name = actor->GetSpriteName();
	int sprite_id = actor->GetSpriteIndex();
	BitmapRef bm = Cache::Charset(sprite_name);
	int width = bm->GetWidth() / 4 / 3;
	int height = bm->GetHeight() / 2 / 4;
	for (int j = 0; j < 3; j++) {
		int sx = ((sprite_id % 4) * 3 + j) * width;
		int sy = ((sprite_id / 4) * 4 + 2) * height;
		Rect src(sx, sy, width, height);
		for (int k = 0; k < 2; k++) {
			BitmapRef bm2 = Bitmap::Create(width, height, true);
			bm2->Clear();
			bm2->Blit(0, 0, *bm, src, 255);
			if (k == 0)
				bm2->ToneBlit(0, 0, *bm2, bm2->GetRect(), Tone(128, 128, 128, 0), Opacity::opaque);
			bitmaps[party_index][j][k] = bm2;
		}
	}
}
