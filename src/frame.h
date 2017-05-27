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

#ifndef _EASYRPG_GRAPHIC_FRAME_H_
#define _EASYRPG_GRAPHIC_FRAME_H_

// Headers
#include <string>
#include "drawable.h"
#include "system.h"

/**
 * Renders the frame overlay.
 */
class Frame : public Drawable {
public:
	Frame();
	~Frame() override;

	void Draw() override;
	void Update();

	int GetZ() const override;
	DrawableType GetType() const override;

private:

	static const int z = Priority_Frame;
	static const DrawableType type = TypeFrame;

	void OnFrameGraphicReady(FileRequestResult* result);

	BitmapRef frame_bitmap;

	FileRequestBinding request_id;
};

#endif
