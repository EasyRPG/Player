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

#include <sstream>

#include "message_overlay.h"
#include "player.h"
#include "graphics.h"
#include "bitmap.h"

MessageOverlay::MessageOverlay() :
	type(TypeOverlay),
	z(100),
	ox(0),
	oy(0),
	text_height(12),
	message_max(10),
	dirty(false),
	counter(0),
	show_all(false) {
	
	black = Bitmap::Create(DisplayUi->GetWidth(), text_height, Color());

	bitmap = Bitmap::Create(DisplayUi->GetWidth(), text_height * message_max, true);

	Graphics::RegisterDrawable(this);
}

MessageOverlay::~MessageOverlay() {
	Graphics::RemoveDrawable(this);
}

bool MessageOverlay::IsGlobal() const {
	return true;
}

void MessageOverlay::Draw() {
	std::deque<MessageOverlayItem>::iterator it;

	if (IsAnyMessageVisible()) {
		++counter;
		if (counter > 150) {
			counter = 0;
			for (it = messages.begin(); it != messages.end(); ++it) {
				if (!it->hidden) {
					it->hidden = true;
					break;
				}
			}
			dirty = true;
		}
	} else if (!show_all) {
		// Don't render overlay when no message visible
		return;
	}

	DisplayUi->GetDisplaySurface()->Blit(ox, oy, *bitmap, bitmap->GetRect(), 255);

	if (!dirty) return;

	bitmap->Clear();

	int i = 0;

	for (it = messages.begin(); it != messages.end(); ++it) {
		if (!it->hidden || show_all) {
			bitmap->Blit(0, i * text_height, *black, black->GetRect(), 128);
			bitmap->TextDraw(2, i * text_height, bitmap->GetWidth(), text_height,
				it->color, it->text);
			++i;
		}
	}

	dirty = false;
}

int MessageOverlay::GetZ() const {
	return z;
}

DrawableType MessageOverlay::GetType() const {
	return type;
}

void MessageOverlay::AddMessage(const std::string& message, Color color) {
	std::stringstream smessage (message);
	std::vector<std::string> strs;
	std::string str;
	while (getline(smessage, str))
		strs.push_back(str);

	for (size_t i = 0; i < strs.size(); i++)
		messages.push_back(MessageOverlayItem(strs[i], color));

	while (messages.size() > (unsigned)message_max) {
		messages.pop_front();
	}
	dirty = true;
}

void MessageOverlay::SetShowAll(bool show_all) {
	this->show_all = show_all;
	dirty = true;
}

bool MessageOverlay::IsAnyMessageVisible() const {
	return std::any_of(messages.cbegin(), messages.cend(), [](const MessageOverlayItem& m) { return !m.hidden; });
}

MessageOverlayItem::MessageOverlayItem(const std::string& text, Color color) :
	text(text), color(color), hidden(false) {
	// no-op
}
