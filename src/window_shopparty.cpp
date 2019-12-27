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
#include "output.h"
#include "reader_util.h"
#include "sprite_character.h"

Window_ShopParty::Window_ShopParty(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetBorderX(4);
	SetContents(Bitmap::Create(width - GetBorderX() * 2, height - 16));

	cycle = 0;
	item_id = 0;

	const std::vector<Game_Actor*>& actors = Main_Data::game_party->GetActors();
	for (size_t i = 0; i < actors.size() && i < 4; i++) {
		const std::string& sprite_name = actors[i]->GetSpriteName();
		FileRequestAsync* request = AsyncHandler::RequestFile("CharSet", sprite_name);
		request->SetGraphicFile(true);
		request_ids.push_back(request->Bind(&Window_ShopParty::OnCharsetSpriteReady, this, (int)i));
		request->Start();
	}

	Refresh();
}

static int CalcEquipScore(const RPG::Item* item) {
	if (item == nullptr) {
		return 0;
	}
	return item->atk_points1 + item->def_points1 + item->agi_points1 + item->spi_points1;
}

static int CmpEquip(const Game_Actor* actor, const RPG::Item* new_item) {
	int new_score = CalcEquipScore(new_item);
	switch (new_item->type) {
	case RPG::Item::Type_weapon:
	{
		int score = CalcEquipScore(actor->GetWeapon());
		if (actor->HasTwoWeapons()) {
			int score2 = CalcEquipScore(actor->Get2ndWeapon());
			score = (new_item->two_handed) ? score + score2 : std::min(score, score2);
		}
		return new_score - score;
	}
	case RPG::Item::Type_helmet:
		return new_score - CalcEquipScore(actor->GetHelmet());
	case RPG::Item::Type_shield:
		return new_score - CalcEquipScore(actor->GetShield());
	case RPG::Item::Type_armor:
		return new_score - CalcEquipScore(actor->GetArmor());
	case RPG::Item::Type_accessory:
		return new_score - CalcEquipScore(actor->GetAccessory());
	default:
		break;
	}
	return 0;
}

static bool IsEquipment(const RPG::Item* item) {
	return item->type == RPG::Item::Type_weapon
		|| item->type == RPG::Item::Type_shield
		|| item->type == RPG::Item::Type_helmet
		|| item->type == RPG::Item::Type_armor
		|| item->type == RPG::Item::Type_accessory;
}

void Window_ShopParty::Refresh() {
	contents->Clear();

	BitmapRef system = Cache::SystemOrBlack();

	if (item_id < 0 || item_id > static_cast<int>(Data::items.size()))
		return;

	const std::vector<Game_Actor*>& actors = Main_Data::game_party->GetActors();
	for (int i = 0; i < static_cast<int>(actors.size()) && i < 4; i++) {
		Game_Actor *actor = actors[i];
		int phase = (cycle / anim_rate) % 4;
		if (phase == 3) {
			phase = 1;
		}
		// RPG_RT displays the actors in an empty shop.
		bool usable = item_id == 0 || actor->IsEquippable(item_id);
		BitmapRef bm = bitmaps[i][usable ? phase : 1][usable ? 1 : 0];

		if (bm) {
			contents->Blit(i * 32, 0, *bm, bm->GetRect(), 255);
		}

		const auto* new_item = ReaderUtil::GetElement(Data::items, item_id);
		if (new_item == nullptr) {
			// Can be null for an empty shop, in which case there is only 1 item 0.
			return;
		}

		bool equippable = usable && IsEquipment(new_item);

		if (equippable) {
			// check if item is equipped by each member
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
				int cmp = CmpEquip(actor, new_item);
				if (cmp > 0) {
					contents->Blit(i * 32 + 20, 24, *system, Rect(128 + 8 * phase, 0, 8, 8), 255);
				}
				else if (cmp < 0) {
					contents->Blit(i * 32 + 20, 24, *system, Rect(128 + 8 * phase, 16, 8, 8), 255);
				}
				else {
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
	auto rect = Sprite_Character::GetCharacterRect(sprite_name, sprite_id, bm->GetRect());
	int width = rect.width / 3;
	int height = rect.height / 4;
	for (int j = 0; j < 3; j++) {
		int sx = ((sprite_id % 4) * 3 + j) * width;
		int sy = ((sprite_id / 4) * 4 + 2) * height;
		Rect src(sx, sy, width, height);
		for (int k = 0; k < 2; k++) {
			BitmapRef bm2 = Bitmap::Create(width, height, true);
			bm2->Clear();
			bm2->Blit(0, 0, *bm, src, 255);
			if (k == 0)
				bm2->ToneBlit(0, 0, *bm2, bm2->GetRect(), Tone(128, 128, 128, 0), Opacity::Opaque());
			bitmaps[party_index][j][k] = bm2;
		}
	}
}
