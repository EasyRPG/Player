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
#include <string>
#include "input.h"
#include "scene.h"
#include "scene_shop.h"
#include "game_system.h"
#include "game_message.h"
#include "window_shop.h"
#include "bitmap.h"
#include "font.h"

Window_Shop::Window_Shop(int shop_type, int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	switch (shop_type) {
		case 0:
			greeting = lcf::Data::terms.shop_greeting1;
			regreeting = lcf::Data::terms.shop_regreeting1;
			buy_msg = lcf::Data::terms.shop_buy1;
			sell_msg = lcf::Data::terms.shop_sell1;
			leave_msg = lcf::Data::terms.shop_leave1;
			buy_select = lcf::Data::terms.shop_buy_select1;
			buy_number = lcf::Data::terms.shop_buy_number1;
			purchased = lcf::Data::terms.shop_purchased1;
			sell_select = lcf::Data::terms.shop_sell_select1;
			sell_number = lcf::Data::terms.shop_sell_number1;
			sold_msg = lcf::Data::terms.shop_sold1;
			break;
		case 1:
			greeting = lcf::Data::terms.shop_greeting2;
			regreeting = lcf::Data::terms.shop_regreeting2;
			buy_msg = lcf::Data::terms.shop_buy2;
			sell_msg = lcf::Data::terms.shop_sell2;
			leave_msg = lcf::Data::terms.shop_leave2;
			buy_select = lcf::Data::terms.shop_buy_select2;
			buy_number = lcf::Data::terms.shop_buy_number2;
			purchased = lcf::Data::terms.shop_purchased2;
			sell_select = lcf::Data::terms.shop_sell_select2;
			sell_number = lcf::Data::terms.shop_sell_number2;
			sold_msg = lcf::Data::terms.shop_sold2;
			break;
		case 2:
			greeting = lcf::Data::terms.shop_greeting3;
			regreeting = lcf::Data::terms.shop_regreeting3;
			buy_msg = lcf::Data::terms.shop_buy3;
			sell_msg = lcf::Data::terms.shop_sell3;
			leave_msg = lcf::Data::terms.shop_leave3;
			buy_select = lcf::Data::terms.shop_buy_select3;
			buy_number = lcf::Data::terms.shop_buy_number3;
			purchased = lcf::Data::terms.shop_purchased3;
			sell_select = lcf::Data::terms.shop_sell_select3;
			sell_number = lcf::Data::terms.shop_sell_number3;
			sold_msg = lcf::Data::terms.shop_sold3;
			break;
	}

	mode = Scene_Shop::BuySellLeave;
	index = 1;

	Refresh();
}

void Window_Shop::UpdateCursorRect() {
	int x = 4;
	int width = contents->GetWidth() - 8;
	if (!Main_Data::game_system->GetMessageFaceName().empty()) {
		if (!Main_Data::game_system->IsMessageFaceRightPosition()) {
			x += LeftMargin + FaceSize + RightFaceMargin;
		}
		width -= LeftMargin + FaceSize + RightFaceMargin;
	}

	Rect rect;
	switch (mode) {
		case Scene_Shop::BuySellLeave:
		case Scene_Shop::BuySellLeave2:
			rect = Rect(x, index * 16, width, 16);
			break;
		default:
			rect = Rect();
			break;
	}

	SetCursorRect(rect);
}

void Window_Shop::Refresh() {
	contents->Clear();

	int x = 0;
	if (!Main_Data::game_system->GetMessageFaceName().empty()) {
		if (!Main_Data::game_system->IsMessageFaceRightPosition()) {
			x += LeftMargin + FaceSize + RightFaceMargin;
			DrawFace(Main_Data::game_system->GetMessageFaceName(), Main_Data::game_system->GetMessageFaceIndex(), LeftMargin, TopMargin, Main_Data::game_system->IsMessageFaceFlipped());
		}
		else {
			DrawFace(Main_Data::game_system->GetMessageFaceName(), Main_Data::game_system->GetMessageFaceIndex(), 248, TopMargin, Main_Data::game_system->IsMessageFaceFlipped());
		}
	}

	int idx = 0;
	switch (mode) {
		case Scene_Shop::BuySellLeave:
		case Scene_Shop::BuySellLeave2:
			contents->TextDraw(x, 2, Font::ColorDefault,
							   mode == Scene_Shop::BuySellLeave2
							   ? regreeting
							   : greeting);
			idx++;

			contents->TextDraw(x + 12, 2 + idx * 16, Font::ColorDefault, buy_msg);
			buy_index = idx++;

			contents->TextDraw(x + 12, 2 + idx * 16, Font::ColorDefault, sell_msg);
			sell_index = idx++;

			contents->TextDraw(x + 12, 2 + idx * 16, Font::ColorDefault, leave_msg);
			leave_index = idx++;
			break;
		case Scene_Shop::Buy:
			contents->TextDraw(x, 2, Font::ColorDefault, buy_select);
			break;
		case Scene_Shop::BuyHowMany:
			contents->TextDraw(x, 2, Font::ColorDefault, buy_number);
			break;
		case Scene_Shop::Bought:
			contents->TextDraw(x, 2, Font::ColorDefault, purchased);
			break;
		case Scene_Shop::Sell:
			contents->TextDraw(x, 2, Font::ColorDefault, sell_select);
			break;
		case Scene_Shop::SellHowMany:
			contents->TextDraw(x, 2, Font::ColorDefault, sell_number);
			break;
		case Scene_Shop::Sold:
			contents->TextDraw(x, 2, Font::ColorDefault, sold_msg);
			break;
	}

	UpdateCursorRect();
}

void Window_Shop::SetMode(int nmode) {
	mode = nmode;
	Refresh();
}

int Window_Shop::GetChoice() const {
	return choice;
}

void Window_Shop::SetChoice(int nchoice) {
	choice = nchoice;
}

void Window_Shop::Update() {
	Window_Base::Update();

	if (active) {
		switch (mode) {
			case Scene_Shop::BuySellLeave:
			case Scene_Shop::BuySellLeave2:
				if (Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN)) {
					if (index < leave_index) {
						index++;
					}
					else {
						index = 1;
					}
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				}
				if (Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP)) {
					if (index > 1) {
						index--;
					}
					else {
						index = leave_index;
					}
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
				}
				if (Input::IsTriggered(Input::DECISION)) {
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
					if (index == buy_index)
						choice = Scene_Shop::Buy;
					if (index == sell_index)
						choice = Scene_Shop::Sell;
					if (index == leave_index)
						choice = Scene_Shop::Leave;
				}
				break;
		}
	}

	UpdateCursorRect();
}
