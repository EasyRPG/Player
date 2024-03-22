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
#include "window_help.h"
#include "bitmap.h"
#include "font.h"

Window_Help::Window_Help(Scene* parent, int ix, int iy, int iwidth, int iheight, Drawable::Flags flags) :
	Window_Base(parent, ix, iy, iwidth, iheight, flags),
	align(Text::AlignLeft) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	contents->Clear();
}

void Window_Help::SetText(std::string text, int color, Text::Alignment align, bool halfwidthspace) {
	if (this->text != text || this->color != color || this->align != align) {
		contents->Clear();

		text_x_offset = 0;
		AddText(text, color, align, halfwidthspace);

		this->text = std::move(text);
		this->align = align;
		this->color = color;
	}
}

void Window_Help::Clear() {
	this->text = "";
	this->color = Font::ColorDefault;
	this->align = Text::AlignLeft;
	text_x_offset = 0;
	contents->Clear();
}

void Window_Help::AddText(std::string text, int color, Text::Alignment align, bool halfwidthspace) {
	std::string::size_type pos = 0;
	std::string::size_type nextpos = 0;
	while (nextpos != std::string::npos) {
		nextpos = text.find(' ', pos);
		auto segment = ToStringView(text).substr(pos, nextpos - pos);
		auto offset = contents->TextDraw(text_x_offset, 2, color, segment, align);
		text_x_offset += offset.x;

		// Special handling for proportional fonts: If the "normal" space is already small do not half it again
		if (nextpos != decltype(text)::npos) {
			int space_width = Text::GetSize(*Font::Default(), " ").width;

			if (halfwidthspace && space_width >= 6) {
				text_x_offset += space_width / 2;
			} else {
				text_x_offset += space_width;
			}
			pos = nextpos + 1;
		}
	}
}
