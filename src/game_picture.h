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

#ifndef EP_GAME_PICTURE_H
#define EP_GAME_PICTURE_H

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
		// RPG Maker 2k3 1.12
		int spritesheet_cols = 1;
		int spritesheet_rows = 1;
		bool spritesheet_loop = false;
		int spritesheet_frame = 0;
		int spritesheet_speed = 0;
		int map_layer = 7;
		int battle_layer = 0;
		// erase_on_map_change | affected_by_flash | affected_by_shake
		int flags = 1 | 32 | 64;
	};

	struct MoveParams : Params {
		int duration;
	};

	void Show(const ShowParams& params);
	void Move(const MoveParams& params);
	void Erase(bool force_erase);

	void Update();

private:
	int id;
	std::unique_ptr<Sprite> sprite;
	BitmapRef whole_bitmap;
	BitmapRef sheet_bitmap;
	int last_spritesheet_frame = 0;
	FileRequestBinding request_id;
	int old_map_x;
	int old_map_y;

	void UpdateSprite();
	void SetNonEffectParams(const Params& params);
	void SyncCurrentToFinish();
	void RequestPictureSprite();
	void OnPictureSpriteReady(FileRequestResult*);
	bool HasSpritesheet() const;
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
