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

#ifndef EP_GRAPHICS_H
#define EP_GRAPHICS_H

#include <memory>
#include <vector>
#include "bitmap.h"
#include "drawable.h"
#include "drawable_list.h"
#include "game_clock.h"

class MessageOverlay;
class Scene;

/**
 * Graphics namespace.
 * Handles screen drawing.
 */
namespace Graphics {
	/**
	 * Controls viewport scaling and positioning
	 */
	class ZoomData {
	private:
		int scale_percentage = 100;
		bool origin_type = true; // true = percentage, false = absolute pixels
		int origin_x = 50; // center point for scaling
		int origin_y = 50; // center point for scaling

	public:
		int GetScale() const { return scale_percentage; }
		void SetScale(int percentage) { scale_percentage = percentage; }

		bool IsOriginPercentage() const { return origin_type; }
		void SetOriginAsPercentage(bool is_relative) { origin_type = is_relative; }

		int GetOriginX() const { return origin_x; }
		void SetOriginX(int x) { origin_x = x; }

		int GetOriginY() const { return origin_y; }
		void SetOriginY(int y) { origin_y = y; }
	};

	// Global instance declaration
	extern ZoomData& GetZoomData();

	/**
	 * Initializes Graphics.
	 */
	void Init();

	/**
	 * Disposes Graphics.
	 */
	void Quit();

	/**
	 * Updates the screen.
	 */
	void Update();

	void Draw(Bitmap& dst);

	void LocalDraw(Bitmap& dst, Drawable::Z_t min_z, Drawable::Z_t max_z);

	std::shared_ptr<Scene> UpdateSceneCallback();

	/**
	 * Returns a handle to the message overlay.
	 * Only used by Output to put messages.
	 *
	 * @return message overlay
	 */
	MessageOverlay& GetMessageOverlay();
}

#endif
