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

#ifndef EP_FPS_OVERLAY_H
#define EP_FPS_OVERLAY_H

#include <deque>
#include <string>
#include "drawable.h"
#include "memory_management.h"
#include "rect.h"
#include "game_clock.h"

/**
 * FpsOverlay class.
 * Shows current FPS and the speedup indicator.
 */
class FpsOverlay : public Drawable {
public:
	FpsOverlay();

	void Draw(Bitmap& dst) override;

	/**
	 * Update the fps overlay.
	 *
	 * @return true if the fps string was changed
	 */
	bool Update();

	/**
	 * Formats a string containing FPS.
	 *
	 * @return fps string
	 */
	std::string GetFpsString() const;

	/**
	 * Set whether we will render the fps.
	 *
	 * @param value true if we want to draw to screen
	 */
	void SetDrawFps(bool value);

private:
	void UpdateText();

	BitmapRef fps_bitmap;
	BitmapRef speedup_bitmap;
	Game_Clock::time_point last_refresh_time;

	/** Rect to draw on screen */
	Rect fps_rect;
	Rect speedup_rect;

	std::string text;

	int last_speed_mod = 1;
	bool speedup_dirty = true;
	bool fps_dirty = true;
	bool draw_fps = true;
};

inline std::string FpsOverlay::GetFpsString() const {
	return text;
}

inline void FpsOverlay::SetDrawFps(bool value) {
	draw_fps = value;
}

#endif
