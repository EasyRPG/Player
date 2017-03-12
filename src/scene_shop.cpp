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
#include "game_temp.h"
#include "game_system.h"
#include "game_party.h"
#include "input.h"
#include "scene_shop.h"
#include "output.h"

Scene_Shop::Scene_Shop() {
	Scene::type = Scene::Shop;
}

void Scene_Shop::Start() {
	shop_window.reset(new Window_Shop(0, (SCREEN_TARGET_WIDTH/2), SCREEN_TARGET_WIDTH, 80));
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	gold_window.reset(new Window_Gold(184, 128, 136, 32));
	empty_window.reset(new Window_Base(0, 32, SCREEN_TARGET_WIDTH, 128));
	empty_window2.reset(new Window_Base(0, 32, 184, 128));
	buy_window.reset(new Window_ShopBuy(0, 32, 184, 128));
	party_window.reset(new Window_ShopParty(184, 32, 136, 48));
	sell_window.reset(new Window_ShopSell(0, 32, SCREEN_TARGET_WIDTH, 128));
	status_window.reset(new Window_ShopStatus(184, 80, 136, 48));
	number_window.reset(new Window_ShopNumber(0, 32, 184, 128));

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

	Game_Temp::shop_transaction = false;
	timer = 0;

	if (Game_Temp::shop_buys && Game_Temp::shop_sells) {
		SetMode(BuySellLeave);
	} else if (Game_Temp::shop_buys) {
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

void Scene_Shop::Update() {
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
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
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
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		if (Game_Temp::shop_sells) {
			SetMode(BuySellLeave2);
		} else {
			Scene::Pop();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		int item_id = buy_window->GetItemId();

		//checks the money and number of items possessed before buy
		if (buy_window->CheckEnable(item_id)) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

			RPG::Item& item = Data::items[item_id - 1];

			int max;
			if (item.price == 0) {
				max = 99;
			} else {
				max = Main_Data::game_party->GetGold() / item.price;
			}
			number_window->SetData(item_id, max, item.price);

			SetMode(BuyHowMany);
		}
		else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
	}
}

void Scene_Shop::UpdateSellSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		if (Game_Temp::shop_buys) {
			SetMode(BuySellLeave2);
		} else {
			Scene::Pop();
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		int item_id = sell_window->GetItem() == NULL ? 0 : sell_window->GetItem()->ID;
		status_window->SetItemId(item_id);
		party_window->SetItemId(item_id);

		if (item_id > 0 && Data::items[item_id - 1].price > 0) {
			RPG::Item& item = Data::items[item_id - 1];
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			number_window->SetData(item_id, Main_Data::game_party->GetItemCount(item_id), item.price / 2);
			SetMode(SellHowMany);
		}
		else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
	}
}

void Scene_Shop::UpdateNumberInput() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
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
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

		Game_Temp::shop_transaction = true;
	}
}
