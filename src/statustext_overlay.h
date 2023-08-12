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

#ifndef EP_STATUSTEXT_OVERLAY_H
#define EP_STATUSTEXT_OVERLAY_H

#include <string>
#include "drawable.h"
#include "memory_management.h"
#include "rect.h"
#include "game_clock.h"

class StatusTextOverlay : public Drawable {
public:
	StatusTextOverlay();

	void ShowText(const std::string& statustext);

	void Draw(Bitmap& dst) override;

	void Update();

private:
	void UpdateText();

	BitmapRef statustext_bitmap;

	Game_Clock::time_point last_update_time;

	Rect statustext_rect;

	std::string text;

	bool show = false;

	bool dirty = true;
};

#endif
