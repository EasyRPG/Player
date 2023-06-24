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
#include <sstream>
#include "game_system.h"
#include "input.h"
#include "util_macro.h"
#include "window_shopnumber.h"
#include "bitmap.h"
#include "font.h"
#include <lcf/reader_util.h>
#include <baseui.h>
#include <output.h>

Window_ShopNumber::Window_ShopNumber(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight),
	item_max(1), price(0), number(1), item_id(0) {

	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->Clear();
}

void Window_ShopNumber::SetData(int item_id, int item_max, int price) {
	this->item_id = item_id;
	this->item_max = item_max;
	this->price = price;
	number = 1;
}

void Window_ShopNumber::Refresh() {
	contents->Clear();

	int y = 34;
	// (Shop) items are guaranteed to be valid
	DrawItemName(*lcf::ReaderUtil::GetElement(lcf::Data::items, item_id), 0, y);

	std::stringstream ss;
	ss << number;

	contents->TextDraw(132, y, Font::ColorDefault, "x");
	contents->TextDraw(132 + 30, y, Font::ColorDefault, ss.str(), Text::AlignRight);
	if (item_max >= 100) {
		SetCursorRect(Rect(132 + 8, y - 2, 26, 16));
	} else {
		SetCursorRect(Rect(132 + 14, y - 2, 20, 16));
	}

	DrawCurrencyValue(GetTotal(), contents->GetWidth(), y + 32);

	if (Input::GetUseMouseButton()) {

		Rect src_rectUp(40, 8, 16, 8);
		contents->Blit(132 + 16, y + 3 - 16, *windowskin, src_rectUp, 255);

		Rect src_rectDown(40, 16, 16, 8);
		contents->Blit(132 + 16, y + 3 + 16, *windowskin, src_rectDown, 255);

	}
}

int Window_ShopNumber::GetNumber() const {
	return number;
}

void Window_ShopNumber::Update() {
	Window_Base::Update();

	int last_number = number;

	if (Input::GetUseMouseButton() && IsVisible()) {
		Point mouseP = Input::GetMousePosition();

		if (Input::IsPressed(Input::MOUSE_LEFT)) {
			disabledByMouse = true;
		}
		
		// Up Arrow
		if (mouseP.x >= GetX() + GetBorderX() + 132 + 14 && mouseP.x <= GetX() + GetBorderX() + 132 + 14 + 14 &&
			mouseP.y >= GetY() + GetBorderY() + 37 - 16 && mouseP.y < GetY() + GetBorderY() + 37 - 16 + 8) {

			// Change cursor (Hand)
			DisplayUi->ChangeCursor(1);

			if (Input::IsRepeated(Input::MOUSE_LEFT) && number < item_max) {
				number++;
			}
		}

		// Down Arrow
		if (mouseP.x >= GetX() + GetBorderX() + 132 + 14 && mouseP.x <= GetX() + GetBorderX() + 132 + 14 + 14 &&
			mouseP.y >= GetY() + GetBorderY() + 37 + 16 && mouseP.y < GetY() + GetBorderY() + 37 + 16 + 8) {

			// Change cursor (Hand)
			DisplayUi->ChangeCursor(1);

			if (Input::IsRepeated(Input::MOUSE_LEFT) && number > 1) {
				number--;
			}
		}

		// Validate
		if (mouseP.x >= GetX() + GetBorderX() + GetCursorRect().x && mouseP.x <= GetX() + GetBorderX() + GetCursorRect().x + GetCursorRect().width &&
			mouseP.y >= GetY() + GetBorderY() + GetCursorRect().y && mouseP.y < GetY() + GetBorderY() + GetCursorRect().y + GetCursorRect().height) {

			// Change cursor (Hand)
			DisplayUi->ChangeCursor(1);

			if (Input::IsReleased(Input::MOUSE_LEFT)) {
				disabledByMouse = false;
			}
		}


		if (Input::IsTriggered(Input::DECISION) && (!Input::IsReleased(Input::MOUSE_LEFT))) {
			disabledByMouse = false;
		}
	}

	if (active) {
		if (Input::IsRepeated(Input::RIGHT) && number < item_max) {
			number++;
		} else if (Input::IsRepeated(Input::LEFT) && number > 1) {
			number--;
		} else if ((Input::IsRepeated(Input::UP) || Input::IsTriggered(Input::SCROLL_UP))
			&& number < item_max) {
			number = min(number + 10, item_max);
		} else if ((Input::IsRepeated(Input::DOWN) || Input::IsTriggered(Input::SCROLL_DOWN))
			&& number > 1) {
			number = max(number - 10, 1);
		}

		if (last_number != number) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
			Refresh();
		}
	}
}

int Window_ShopNumber::GetTotal() const {
	return price * number;
}
