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
#include "game_temp.h"
#include "game_system.h"
#include "game_party.h"
#include "input.h"
#include "scene_shop.h"

////////////////////////////////////////////////////////////
Scene_Shop::Scene_Shop() :
	help_window(NULL),
	buy_window(NULL),
	party_window(NULL),
	status_window(NULL),
	gold_window(NULL),
	sell_window(NULL),
	count_window(NULL),
	empty2_window(NULL),
	empty_window(NULL),
	shop_window(NULL) {
	Scene::type = Scene::Shop;
}

////////////////////////////////////////////////////////////
void Scene_Shop::Start() {
	// Create the windows

	help_window = new Window_Help(0, 0, 320, 32);
	buy_window = new Window_ShopBuy(0, 32, 184, 128);
	party_window = new Window_Party(184, 32, 136, 48);
	status_window = new Window_ShopStatus(184, 80, 136, 48);
	gold_window = new Window_Gold(184, 128, 136, 32);
	sell_window = new Window_ShopSell(0, 32, 320, 128);
	count_window = new Window_ShopNumber(0, 32, 184, 128);
	empty2_window = new Window_Base(0, 32, 184, 128);
	empty_window = new Window_Base(0, 32, 320, 128);
	shop_window = new Window_Shop(0, 160, 320, 80);

	help_window->SetText("");
	help_window->SetVisible(true);

	shop_window->SetVisible(true);

	buy_window->SetHelpWindow(help_window);
	buy_window->SetStatusWindow(status_window);
	buy_window->SetPartyWindow(party_window);

	sell_window->SetHelpWindow(help_window);
	sell_window->SetPartyWindow(party_window);
	sell_window->SetActive(false);
	sell_window->Refresh();
	sell_window->SetIndex(0);
	sell_window->SetActive(true);

	Game_Temp::shop_transaction = false;
	timer = 0;

	SetMode(BuySellLeave);
}

////////////////////////////////////////////////////////////
void Scene_Shop::Terminate() {
	delete help_window;
	delete buy_window;
	delete party_window;
	delete status_window;
	delete gold_window;
	delete sell_window;
	delete count_window;
	delete empty2_window;
	delete empty_window;
	delete shop_window;
}

////////////////////////////////////////////////////////////
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
			Enable(empty_window, true);
			Enable(sell_window, false);
			break;
		case Sell:
			Enable(empty_window, false);
			Enable(sell_window, true);
			break;
		case Buy:
		case BuyHowMany:
		case SellHowMany:
		case Bought:
		case Sold:
			Enable(empty_window, false);
			Enable(sell_window, false);
			break;
	}

	// Right-hand panels
	switch (mode) {
		case BuySellLeave:
		case BuySellLeave2:
		case Sell:
			Enable(party_window, false);
			Enable(status_window, false);
			Enable(gold_window, false);
			break;
		case Buy:
		case BuyHowMany:
		case SellHowMany:
		case Bought:
		case Sold:
			Enable(party_window, true);
			Enable(status_window, true);
			Enable(gold_window, true);
			break;
	}

	// Left-hand panels
	switch (mode) {
		case BuySellLeave:
		case BuySellLeave2:
		case Sell:
			Enable(buy_window, false);
			Enable(count_window, false);
			Enable(empty2_window, false);
			break;
		case Buy:
			Enable(buy_window, true);
			Enable(count_window, false);
			Enable(empty2_window, false);
			break;
		case BuyHowMany:
		case SellHowMany:
			Enable(buy_window, false);
			Enable(count_window, true);
			Enable(empty2_window, false);
			break;
		case Bought:
		case Sold:
			Enable(buy_window, false);
			Enable(count_window, false);
			Enable(empty2_window, true);
			timer = DEFAULT_FPS;
			break;
	}

	shop_window->SetMode(mode);
}

////////////////////////////////////////////////////////////
void Scene_Shop::Update() {
	buy_window->Update();
	sell_window->Update();
	shop_window->Update();
	count_window->Update();
	party_window->Update();

	switch (mode) {
		case BuySellLeave:
		case BuySellLeave2:
			if (Input::IsTriggered(Input::DECISION)) {
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
			if (Input::IsTriggered(Input::CANCEL)) {
				Game_System::SePlay(Data::system.cancel_se);
				Scene::Pop();
			}
			break;
		case Buy:
			if (Input::IsTriggered(Input::CANCEL)) {
				Game_System::SePlay(Data::system.cancel_se);
				SetMode(BuySellLeave2);
			}
			if (Input::IsTriggered(Input::DECISION)) {
				int item_id = buy_window->GetSelected();
				//checks the money and number of items possessed before buy
				if (buy_window->CheckEnable(item_id)) {
					Game_System::SePlay(Data::system.decision_se);
					RPG::Item& item = Data::items[item_id - 1];
					int value = item.price;
					int limit = std::min(99, Game_Party::GetGold() / value);
					const std::string& name = item.name;
					count_window->SetRange(1, limit);
					count_window->SetNumber(1);
					count_window->SetItemValue(value);
					count_window->SetItemName(name);
					SetMode(BuyHowMany);
				}
				else {
					Game_System::SePlay(Data::system.buzzer_se);
				}
			}
			break;
		case Sell:
			if (Input::IsTriggered(Input::CANCEL)) {
				Game_System::SePlay(Data::system.cancel_se);
				SetMode(BuySellLeave2);
			}
			if (Input::IsTriggered(Input::DECISION)) {
				int item_id = sell_window->GetItemId();
				// checks if the item has a valid id
				if (item_id > 0) {
					Game_System::SePlay(Data::system.decision_se);
					RPG::Item& item = Data::items[item_id - 1];
					int value = item.price;
					const std::string& name = item.name;
					int possessed = Game_Party::ItemNumber(item_id);
					count_window->SetRange(1, possessed);
					count_window->SetNumber(1);
					count_window->SetItemValue(value);
					count_window->SetItemName(name);
					SetMode(SellHowMany);
				}
				else {
					Game_System::SePlay(Data::system.buzzer_se);
				}
				
			}
			break;
		case BuyHowMany:
			if (Input::IsTriggered(Input::CANCEL)) {
				Game_System::SePlay(Data::system.cancel_se);
				SetMode(Buy);
			}
			if (Input::IsTriggered(Input::DECISION)) {
				Game_System::SePlay(Data::system.decision_se);
				int item_id = buy_window->GetSelected();
				Game_Party::GainGold(-count_window->GetTotal());
				Game_Party::GainItem(item_id, count_window->GetNumber());
				gold_window->Refresh();
				sell_window->Refresh();
				Game_Temp::shop_transaction = true;
				SetMode(Buy);
			}
			break;
		case SellHowMany:
			if (Input::IsTriggered(Input::CANCEL)) {
				Game_System::SePlay(Data::system.cancel_se);
				SetMode(Sell);
			}
			if (Input::IsTriggered(Input::DECISION)) {
				Game_System::SePlay(Data::system.decision_se);
				int item_id = sell_window->GetItemId();
				Game_Party::GainGold(count_window->GetTotal());
				Game_Party::LoseItem(item_id, count_window->GetNumber());
				gold_window->Refresh();
				sell_window->Refresh();
				Game_Temp::shop_transaction = true;
				SetMode(Sell);
			}
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
			return;
	}
}
