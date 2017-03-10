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

#ifndef _PICTURE_H_
#define _PICTURE_H_

// Headers
#include <string>
#include "async_handler.h"
#include "rpg_save.h"
#include "sprite.h"

class Sprite;

/**
 * Picture class.
 */
class Game_Picture {
public:
	explicit Game_Picture(int ID);
	~Game_Picture();

	struct Params {
		int position_x;
		int position_y;
		int magnify;
		int top_trans;
		int bottom_trans;
		int red;
		int green;
		int blue;
		int saturation;
		int effect_mode;
		int effect_power;
	};
	struct ShowParams : Params {
		std::string name;
		bool transparency;
		bool fixed_to_map;
	};
	struct MoveParams : Params {
		int duration;
	};

	void Show(const ShowParams& params);
	void Move(const MoveParams& params);
	void Erase();

	void Update();

private:
	int id;
	std::unique_ptr<Sprite> sprite;
	FileRequestBinding request_id;
	int old_map_x;
	int old_map_y;

	void UpdateSprite();
	void SetNonEffectParams(const Params& params);
	void SyncCurrentToFinish();
	void RequestPictureSprite();
	void OnPictureSpriteReady(FileRequestResult*);
	/**
	 * Compared to other classes picture doesn't hold a direct reference.
	 * Resizing the picture vector when the ID is larger then the vector can
	 * result in a memmove on resize, resulting in data pointers pointing into
	 * garbage.
	 *
	 * @return Reference to the SavePicture data
	 */
	RPG::SavePicture& GetData() const;
};

#endif
