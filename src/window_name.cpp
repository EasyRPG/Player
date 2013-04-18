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
#include "window_name.h"
#include "bitmap.h"
#include "font.h"
#include "data.h"
#include "game_system.h"

#include <boost/regex/pending/unicode_iterator.hpp>

Window_Name::Window_Name(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());

	name.clear();

	Refresh();
}

void Window_Name::Refresh() {
	contents->Clear();
	contents->TextDraw(2, 2, Font::ColorDefault, name);
}

void Window_Name::Set(const std::string& text) {
	name = text;
	Refresh();
}

void Window_Name::Append(const std::string& text) {
	if(contents->GetFont()->GetSize(name + text).width <= (12 * 6)) {
		name += text;
		Refresh();
	} else {
		Game_System::SePlay(Main_Data::game_data.system.buzzer_se);
	}
}

void Window_Name::Update() {
	Rect const name_size = contents->GetFont()->GetSize(name);
	SetCursorRect(Rect(name_size.width + 2, 0, 16, 16));
}

void Window_Name::Erase() {
	if (name.size() < 1)
		return;

	boost::u8_to_u32_iterator<std::string::const_iterator> name_begin =
		boost::u8_to_u32_iterator<std::string::const_iterator>(name.begin(), name.begin(), name.begin());
	boost::u8_to_u32_iterator<std::string::const_iterator> name_end =
		boost::u8_to_u32_iterator<std::string::const_iterator>(name.end(), name.begin(), name.end());
	--name_end;

	name = std::string(name_begin.base(), name_end.base());
	Refresh();
}

const std::string& Window_Name::Get() {
	return name;
}
