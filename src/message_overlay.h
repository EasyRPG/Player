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

#ifndef _MESSAGE_OVERLAY_H_
#define _MESSAGE_OVERLAY_H_

#include <deque>
#include "bitmap.h"
#include "bitmap_screen.h"
#include "color.h"
#include "drawable.h"
#include "tone.h"

class MessageOverlayItem {
public:
	MessageOverlayItem(const std::string& text, Color color);

	std::string text;
	Color color;
	bool hidden;
};

/**
 * MessageOverlay class.
 * Displays notifications during the game session.
 */
class MessageOverlay : public Drawable {
public:
	MessageOverlay();
	virtual ~MessageOverlay();

	void Draw();

	int GetZ() const;

	DrawableType GetType() const;

	bool IsGlobal() const;

	void AddMessage(const std::string& message, Color color);

	void SetShowAll(bool show_all);

private:
	DrawableType type;

	BitmapRef bitmap;
	BitmapRef black;
	BitmapScreenRef bitmap_screen;

	int z;
	int ox;
	int oy;

	int text_height;
	int message_max;

	std::deque<MessageOverlayItem> messages;

	bool dirty;

	int counter;

	bool show_all;
};

#endif
