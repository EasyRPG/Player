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

#ifndef _SCENE_SHOP_H_
#define _SCENE_SHOP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "scene.h"
#include "window_base.h"
#include "window_help.h"
#include "window_buy.h"
#include "window_count.h"
#include "window_party.h"
#include "window_total.h"
#include "window_gold.h"
#include "window_sell.h"
#include "window_shop.h"

////////////////////////////////////////////////////////////
/// Scene_Item class
////////////////////////////////////////////////////////////
class Scene_Shop : public Scene {

public:
	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param item_index : index to select
	////////////////////////////////////////////////////////
	Scene_Shop();

	void Start();
	void Update();
	void Terminate();

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

private:
	/// Displays available items
	Window_Help* help_window;
	Window_Buy* buy_window;
	Window_Party* party_window;
	Window_Total* total_window;
	Window_Gold* gold_window;
	Window_Sell* sell_window;
	Window_Count* count_window;
	Window_Base* empty2_window;
	Window_Base* empty_window;
	Window_Shop* shop_window;
	int mode;
	int timer;
};

#endif
