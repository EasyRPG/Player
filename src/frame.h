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

#ifndef EP_FRAME_H
#define EP_FRAME_H

// Headers
#include <string>
#include "drawable.h"
#include "system.h"
#include "async_handler.h"

/**
 * Renders the frame overlay.
 */
class Frame : public Drawable {
public:
	Frame();

	void Draw(Bitmap& dst) override;
	void Update();

private:
	void OnFrameGraphicReady(FileRequestResult* result);

	BitmapRef frame_bitmap;

	FileRequestBinding request_id;
};

#endif
