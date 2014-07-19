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

#include "message_overlay.h"
#include "player.h"
#include "graphics.h"
#include "bitmap.h"

#include <boost/algorithm/string.hpp>

MessageOverlay::MessageOverlay()
    : type(TypeMessageOverlay)
    , z(100)
    , ox(0)
    , oy(0)
    , text_height(12)
    , message_max(10)
    , dirty(false)
    , counter(0)
    , show_all(false) {

	bitmap_screen = BitmapScreen::Create();
	black = Bitmap::Create(DisplayUi->GetWidth(), text_height, Color());

	bitmap = Bitmap::Create(DisplayUi->GetWidth(), text_height * message_max, true);
	bitmap_screen->SetBitmap(bitmap);

	Graphics::RegisterDrawable(this);
}

MessageOverlay::~MessageOverlay() { Graphics::RemoveDrawable(this); }

bool MessageOverlay::IsGlobal() const { return true; }

void MessageOverlay::Draw() {
	std::deque<MessageOverlayItem>::iterator it;

	++counter;
	if (counter > 150) {
		counter = 0;
		if (!messages.empty()) {
			for (it = messages.begin(); it != messages.end(); ++it) {
				if (!it->hidden) {
					it->hidden = true;
					break;
				}
			}
			dirty = true;
		}
	} else {
		if (!messages.empty()) {
			bitmap_screen->BlitScreen(ox, oy);
		}
	}

	if (!dirty) return;

	bitmap->Clear();

	int i = 0;

	for (it = messages.begin(); it != messages.end(); ++it) {
		if (!it->hidden || show_all) {
			bitmap->Blit(0, i * text_height, *black, black->GetRect(), 128);
			bitmap->TextDraw(2, i * text_height, bitmap->GetWidth(), text_height, it->color,
			                 it->text);
			++i;
		}
	}

	bitmap_screen->BlitScreen(ox, oy);

	dirty = false;
}

int MessageOverlay::GetZ() const { return z; }

DrawableType MessageOverlay::GetType() const { return type; }

void MessageOverlay::AddMessage(const std::string& message, Color color) {
	std::vector<std::string> strs;
	boost::split(strs, message, boost::is_any_of("\n"));

	for (size_t i = 0; i < strs.size(); i++) messages.push_back(MessageOverlayItem(strs[i], color));

	if (messages.size() > (unsigned)message_max) {
		messages.pop_front();
	}
	dirty = true;
}

void MessageOverlay::SetShowAll(bool show_all) {
	this->show_all = show_all;
	dirty = true;
}

MessageOverlayItem::MessageOverlayItem(const std::string& text, Color color)
    : text(text), color(color), hidden(false) {
	// no-op
}
