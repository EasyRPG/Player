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
#include <lcf/data.h>
#include "game_system.h"

Window_Name::Window_Name(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Base(parent, ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	name.clear();

	Refresh();
}

void Window_Name::Refresh() {
	contents->Clear();
	contents->TextDraw(2, 2, Font::ColorDefault, name);
}

void Window_Name::Set(std::string text) {
	name = std::move(text);
	Refresh();
}

void Window_Name::Append(StringView text) {
	// Avoid string copies by reusing the buffer in name
	name.append(text.begin(), text.end());
	if (Text::GetSize(*Font::Default(), name).width <= (12 * 6)) {
		Refresh();
	} else {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		name.resize(name.size() - text.size());
	}
}

void Window_Name::Update() {
	Rect const name_size = Text::GetSize(*Font::Default(), name);
	SetCursorRect(Rect(name_size.width + 2, 0, 16, 16));
}

void Window_Name::Erase() {
	if (name.empty())
		return;

	auto u32name = Utils::DecodeUTF32(name);
	u32name.pop_back();
	name = Utils::EncodeUTF(u32name);

	Refresh();
}

