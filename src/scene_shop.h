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

#ifndef EP_SCENE_SHOP_H
#define EP_SCENE_SHOP_H

// Headers
#include "scene.h"
#include "window_base.h"
#include "window_help.h"
#include "window_gold.h"
#include "window_shopparty.h"
#include "window_shopbuy.h"
#include "window_shopnumber.h"
#include "window_shopstatus.h"
#include "window_shopsell.h"
#include "window_shop.h"

/**
 * Scene Shop class.
 * Manages buying and selling of items.
 */
class Scene_Shop : public Scene {

public:
	using Continuation = std::function<void(bool)>;
	/**
	 * Constructor.
	 */
	Scene_Shop(
		std::vector<int> goods,
		int shop_type,
		bool allow_buy,
		bool allow_sell,
		Continuation on_finish);

	void Start() override;
	void Suspend(SceneType next_scene) override;

	enum ShopMode {
		BuySellLeave,
		BuySellLeave2,
		Buy,
		BuyHowMany,
		Bought,
		Sell,
		SellHowMany,
		Sold,
		Leave
	};

	void SetMode(int nmode);

	void vUpdate() override;
	void UpdateCommandSelection();
	void UpdateBuySelection();
	void UpdateSellSelection();
	void UpdateNumberInput();

private:
	/** Displays available items. */
	std::unique_ptr<Window_Help> help_window;
	std::unique_ptr<Window_ShopBuy> buy_window;
	std::unique_ptr<Window_ShopParty> party_window;
	std::unique_ptr<Window_ShopStatus> status_window;
	std::unique_ptr<Window_Gold> gold_window;
	std::unique_ptr<Window_ShopSell> sell_window;
	std::unique_ptr<Window_ShopNumber> number_window;
	std::unique_ptr<Window_Base> empty_window;
	std::unique_ptr<Window_Base> empty_window2;
	std::unique_ptr<Window_Shop> shop_window;
	Continuation on_finish;

	std::vector<int> goods;
	int shop_type = 0;
	int mode = 0;
	int timer = 0;
	bool allow_buy = false;
	bool allow_sell = false;
	bool did_transaction = false;
};

#endif
