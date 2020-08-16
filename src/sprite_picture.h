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

#ifndef EP_PICTURE_SPRITE_H
#define EP_PICTURE_SPRITE_H

#include "sprite.h"

class Bitmap;

/**
 * Sprite picture class.
 */
class Sprite_Picture : public Sprite {
public:
	/**
	 * Constructor.
	 *
	 * @param pic_id the picture id
	 */
	Sprite_Picture(int pic_id, Drawable::Flags flags = Drawable::Flags::Default);

	void Draw(Bitmap& dst) override;

	void OnPictureShow();

private:
	int last_spritesheet_frame = -1;
	const int pic_id = 0;
	const bool feature_spritesheet = false;
	const bool feature_priority_layers = false;
	const bool feature_bottom_trans = false;
};

#endif
