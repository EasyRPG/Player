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

#ifndef EP_MESSAGE_OVERLAY_H
#define EP_MESSAGE_OVERLAY_H

#include <deque>
#include <string>
#include "color.h"
#include "drawable.h"
#include "memory_management.h"
#include "tone.h"

class MessageOverlayItem {
public:
	MessageOverlayItem(const std::string& text, Color color);

	std::string text;
	Color color;
	bool hidden = false;
	int repeat_count = 0;
};

/**
 * MessageOverlay class.
 * Displays notifications during the game session.
 */
class MessageOverlay : public Drawable {
public:
	MessageOverlay();

	void Draw(Bitmap& dst) override;

	void Update();

	void AddMessage(const std::string& message, Color color);

	void SetShowAll(bool show_all);

	void OnResolutionChange();

private:
	bool IsAnyMessageVisible() const;

	BitmapRef bitmap;
	BitmapRef black;

	int ox = 0;
	int oy = 0;

	int text_height = 12;
	int message_max = 10;

	std::deque<MessageOverlayItem> messages;
	/** Last message added to the console before linebreak processing */
	std::string last_message;

	bool dirty = false;

	int counter = 0;

	bool show_all = false;
};

#endif
