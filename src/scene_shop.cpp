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
#include "game_system.h"
#include "game_party.h"
#include "input.h"
#include <lcf/reader_util.h>
#include "scene_shop.h"
#include "output.h"

Scene_Shop::Scene_Shop(
		std::vector<int> goods,
		int shop_type,
		bool allow_buy,
		bool allow_sell,
		Continuation on_finish)
	: on_finish(std::move(on_finish)),
	goods(std::move(goods)),
	shop_type(shop_type),
	allow_buy(allow_buy),
	allow_sell(allow_sell)
{
	Scene::type = Scene::Shop;
}

void Scene_Shop::Start() {
	// Sanitize shop items
	for (auto it = goods.begin(); it != goods.end();) {
		const auto* item = lcf::ReaderUtil::GetElement(lcf::Data::items, *it);
		if (!item) {
			Output::Warning("Removed invalid item {} from shop", *it);
			it = goods.erase(it);
		} else {
			++it;
		}
	}

	int window_help_height = 32;
	int window_buy_width = 184;
	int window_buy_height = 128;
	int window_party_width = 136;
	int window_party_height = 48;
	int window_status_width = 136;
	int window_status_height = 48;
	int window_gold_width = 136;
	int window_gold_height = 32;
	int window_shop_height = 80;

	help_window.reset(new Window_Help(Player::menu_offset_x, Player::menu_offset_y, MENU_WIDTH, window_help_height));
	buy_window.reset(new Window_ShopBuy(goods, Player::menu_offset_x, Player::menu_offset_y + window_help_height, window_buy_width, window_buy_height));
	number_window.reset(new Window_ShopNumber(Player::menu_offset_x, Player::menu_offset_y + window_help_height, window_buy_width, window_buy_height));
	party_window.reset(new Window_ShopParty(Player::menu_offset_x + window_buy_width, Player::menu_offset_y + window_help_height, window_party_width, window_party_height));
	status_window.reset(new Window_ShopStatus(Player::menu_offset_x + window_buy_width, Player::menu_offset_y + window_help_height + window_party_height, window_status_width, window_status_height));
	gold_window.reset(new Window_Gold(Player::menu_offset_x + window_buy_width, Player::menu_offset_y + window_help_height + window_party_height + window_status_height, window_gold_width, window_gold_height));
	shop_window.reset(new Window_Shop(shop_type, Player::menu_offset_x, Player::menu_offset_y + window_help_height + window_party_height + window_status_height + window_gold_height, MENU_WIDTH, window_shop_height));
	sell_window.reset(new Window_ShopSell(Player::menu_offset_x, Player::menu_offset_y + window_help_height, MENU_WIDTH, window_buy_height));
	empty_window.reset(new Window_Base(Player::menu_offset_x, Player::menu_offset_y + window_help_height, MENU_WIDTH, window_buy_height));
	empty_window2.reset(new Window_Base(Player::menu_offset_x, Player::menu_offset_y + window_help_height, window_buy_width, window_buy_height));

	buy_window->SetActive(false);
	buy_window->SetVisible(false);
	buy_window->SetHelpWindow(help_window.get());

	sell_window->SetActive(false);
	sell_window->SetVisible(false);
	sell_window->SetHelpWindow(help_window.get());

	number_window->SetActive(false);
	number_window->SetVisible(false);

	status_window->SetVisible(false);
	sell_window->Refresh();
	sell_window->SetIndex(0);
	sell_window->SetActive(true);

	timer = 0;

	if (allow_buy && allow_sell) {
		SetMode(BuySellLeave);
	} else if (allow_buy) {
		shop_window->SetChoice(Buy);
		SetMode(Buy);
	} else {
		shop_window->SetChoice(Sell);
		SetMode(Sell);
	}
}

static void Enable(Window* window, bool state) {
	window->SetVisible(state);
	window->SetActive(state);
}

void Scene_Shop::SetMode(int nmode) {
	mode = nmode;
	help_window->SetText("");

	// Central panel
	switch (mode) {
		case BuySellLeave:
		case BuySellLeave2:
			empty_window->SetVisible(true);
			Enable(sell_window.get(), false);
			break;
		case Sell:
			empty_window->SetVisible(false);
			Enable(sell_window.get(), true);
			break;
		case Buy:
		case BuyHowMany:
		case SellHowMany:
		case Bought:
		case Sold:
			empty_window->SetVisible(false);
			Enable(sell_window.get(), false);
			break;
	}

	// Right-hand panels
	switch (mode) {
		case BuySellLeave:
		case BuySellLeave2:
		case Sell:
			party_window->SetVisible(false);
			status_window->SetVisible(false);
			gold_window->SetVisible(false);
			break;
		case Buy:
		case BuyHowMany:
		case SellHowMany:
		case Bought:
		case Sold:
			party_window->SetVisible(true);
			status_window->SetVisible(true);
			gold_window->SetVisible(true);
			break;
	}

	// Left-hand panels
	switch (mode) {
		case BuySellLeave:
		case BuySellLeave2:
			Enable(buy_window.get(), false);
			Enable(number_window.get(), false);
			empty_window2->SetVisible(false);
			break;
		case Sell:
			sell_window->Refresh();
			Enable(number_window.get(), false);
			empty_window2->SetVisible(false);
			break;
		case Buy:
			buy_window->Refresh();
			Enable(buy_window.get(), true);
			Enable(number_window.get(), false);
			empty_window2->SetVisible(false);
			break;
		case BuyHowMany:
		case SellHowMany:
			number_window.get()->Refresh();
			Enable(buy_window.get(), false);
			Enable(number_window.get(), true);
			break;
		case Bought:
		case Sold:
			Enable(buy_window.get(), false);
			Enable(number_window.get(), false);
			empty_window2->SetVisible(true);

			timer = DEFAULT_FPS;
			break;
	}

	shop_window->SetMode(mode);
}

void Scene_Shop::vUpdate() {
	buy_window->Update();
	sell_window->Update();
	shop_window->Update();
	number_window->Update();
	party_window->Update();

	switch (mode) {
		case BuySellLeave:
		case BuySellLeave2:
			UpdateCommandSelection();
			break;
		case Buy:
			UpdateBuySelection();
			break;
		case Sell:
			UpdateSellSelection();
			break;
		case BuyHowMany:
		case SellHowMany:
			UpdateNumberInput();
			break;
		case Bought:
			timer--;
			if (timer == 0)
				SetMode(Buy);
			break;
		case Sold:
			timer--;
			if (timer == 0)
				SetMode(Sell);
			break;
		default:
			break;
	}
}

void Scene_Shop::UpdateCommandSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION) && shop_window->GetIndex() >= 0) {
		switch (shop_window->GetChoice()) {
			case Buy:
			case Sell:
				SetMode(shop_window->GetChoice());
				break;
			case Leave:
				Scene::Pop();
				break;
		}
	}
}

void Scene_Shop::UpdateBuySelection() {
	status_window->SetItemId(buy_window->GetItemId());
	party_window->SetItemId(buy_window->GetItemId());

	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		if (allow_sell) {
			SetMode(BuySellLeave2);
		} else {
			Scene::Pop();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		int item_id = buy_window->GetItemId();

		// checks the money and number of items possessed before buy
		if (buy_window->CheckEnable(item_id)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

			// Items are guaranteed to be valid
			const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);

			int max = Main_Data::game_party->GetMaxItemCount(item_id) - Main_Data::game_party->GetItemCount(item_id);
			if (item->price > 0) {
				max = std::min<int>(max, Main_Data::game_party->GetGold() / item->price);
			}
			number_window->SetData(item_id, max, item->price);

			SetMode(BuyHowMany);
		}
		else if (buy_window->GetIndex() >= 0) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}
	}
}

void Scene_Shop::UpdateSellSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		if (allow_buy) {
			SetMode(BuySellLeave2);
		} else {
			Scene::Pop();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		const lcf::rpg::Item* item = sell_window->GetItem();
		int item_id = (item != nullptr) ? item->ID : 0;
		status_window->SetItemId(item_id);
		party_window->SetItemId(item_id);

		if (item && item->price > 0) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			number_window->SetData(item->ID, Main_Data::game_party->GetItemCount(item->ID), item->price / 2);
			SetMode(SellHowMany);
		}
		else if (sell_window->GetIndex() >= 0) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		}
		
	}
}

void Scene_Shop::UpdateNumberInput() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		switch (shop_window->GetChoice()) {
		case Buy:
			SetMode(Buy); break;
		case Sell:
			SetMode(Sell); break;
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		int item_id;
		switch (shop_window->GetChoice()) {
		case Buy:
			item_id = buy_window->GetItemId();
			Main_Data::game_party->LoseGold(number_window->GetTotal());
			Main_Data::game_party->AddItem(item_id, number_window->GetNumber());
			gold_window->Refresh();
			buy_window->Refresh();
			status_window->Refresh();
			SetMode(Bought); break;
		case Sell:
			item_id = sell_window->GetItem() == NULL ? 0 : sell_window->GetItem()->ID;
			Main_Data::game_party->GainGold(number_window->GetTotal());
			Main_Data::game_party->RemoveItem(item_id, number_window->GetNumber());
			gold_window->Refresh();
			sell_window->Refresh();
			status_window->Refresh();
			SetMode(Sold); break;
		}
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

		did_transaction = true;
	}
}

void Scene_Shop::Suspend(SceneType /* next_scene */) {
	if (on_finish) {
		on_finish(did_transaction);
	}
}
