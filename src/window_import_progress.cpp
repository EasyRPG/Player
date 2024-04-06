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
#include "window_import_progress.h"
#include "bitmap.h"
#include "color.h"
#include "font.h"

Window_ImportProgress::Window_ImportProgress(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Base(parent, ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));
	Refresh();
}

void Window_ImportProgress::SetProgress(int pct, const std::string& path) {
	percent = pct;
	curr_path = path;
	Refresh();
}

void Window_ImportProgress::Refresh() {
	contents->Clear();

	contents->TextDraw(0, 2, Font::ColorDefault, "Searching for files...", Text::AlignLeft);

	Rect inner(1, 16+3, 142, 16-6);
	Rect outer(inner.x-1, inner.y-1, inner.width+2, inner.height+2);

	contents->FillRect(outer, Color(0x00, 0x00, 0x00, 0xFF));
	contents->FillRect(inner, Color(0x66, 0x66, 0x66, 0xFF));
	inner.width = (inner.width*percent) / 100;
	contents->FillRect(inner, Color(0xFF, 0x00, 0x00, 0xFF));

	contents->TextDraw(0, 2 + 32, Font::ColorDefault, std::string("Folder: ") + curr_path, Text::AlignLeft);
}
