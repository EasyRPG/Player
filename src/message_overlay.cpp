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
#include "bitmap.h"
#include "game_message.h"
#include "drawable_mgr.h"
#include "baseui.h"

MessageOverlay::MessageOverlay() : Drawable(Priority_Overlay, Drawable::Flags::Global)
{
	// Graphics::RegisterDrawable is in the Update function
}

void MessageOverlay::Draw(Bitmap& dst) {
	if (!IsAnyMessageVisible() && !show_all) {
		// Don't render overlay when no message visible
		return;
	}

	dst.Blit(ox, oy, *bitmap, bitmap->GetRect(), 255);

	if (!dirty) return;

	bitmap->Clear();

	int i = 0;

	for (auto& message : messages) {
		if (!message.hidden || show_all) {
			bitmap->Blit(0, i * text_height, *black, black->GetRect(), 128);

			std::string text = message.text;
			if (message.repeat_count > 0) {
				text += " [" + std::to_string(message.repeat_count + 1) + "x]";
			}

			Text::Draw(*bitmap, 2, i * text_height, *Font::DefaultBitmapFont(), message.color, text);
			++i;
		}
	}

	dirty = false;
}

void MessageOverlay::AddMessage(const std::string& message, Color color) {
	if (message.empty()) {
		return;
	}

	if (message == last_message) {
		// The message matches the previous message -> increase counter
		messages.back().repeat_count++;
		messages.back().hidden = false;
		// Keep the old message (with a new counter) on the screen
		counter = 0;

		dirty = true;
		return;
	}

	last_message = message;

	Game_Message::WordWrap(
			message,
			Player::screen_width - 6, // hardcoded to screen width because the bitmap is not initialized early enough
			[&](StringView line) {
				messages.emplace_back(std::string(line), color);
			}, *Font::DefaultBitmapFont()
	);

	while (messages.size() > (unsigned)message_max) {
		messages.pop_front();
	}

	dirty = true;
}

void MessageOverlay::Update() {
	if (!DisplayUi) {
		return;
	}

	if (!bitmap) {
		// Initialisation is delayed because the display is not ready on startup
		OnResolutionChange();
	}

	if (IsAnyMessageVisible()) {
		++counter;
		if (counter > 150) {
			counter = 0;
			for (auto& message : messages) {
				if (!message.hidden) {
					message.hidden = true;
					break;
				}
			}
			dirty = true;
		}
	}
}

void MessageOverlay::SetShowAll(bool show_all) {
	this->show_all = show_all;
	dirty = true;
}

void MessageOverlay::OnResolutionChange() {
	if (!bitmap) {
		DrawableMgr::Register(this);
	}

	black = Bitmap::Create(Player::screen_width, text_height, Color(0, 0, 0, 255));
	bitmap = Bitmap::Create(Player::screen_width, text_height * message_max, true);
}

bool MessageOverlay::IsAnyMessageVisible() const {
	return std::any_of(messages.cbegin(), messages.cend(), [](const MessageOverlayItem& m) { return !m.hidden; });
}

MessageOverlayItem::MessageOverlayItem(const std::string& text, Color color) :
	text(text), color(color)
{
	// no-op
}
