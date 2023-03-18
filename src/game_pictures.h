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
#include <deque>
#include "async_handler.h"
#include <lcf/rpg/savepicture.h>
#include "sprite_picture.h"

class Sprite_Picture;
class Scene;
class Window_Base;

/**
 * Pictures class.
 */
class Game_Pictures {
public:
	Game_Pictures() = default;

	void SetSaveData(std::vector<lcf::rpg::SavePicture> save);
	std::vector<lcf::rpg::SavePicture> GetSaveData() const;

	void InitGraphics();

	static int GetDefaultNumberOfPictures();

	struct Params {
		int position_x = 0;
		int position_y = 0;
		int magnify = 100;
		int top_trans = 0;
		int bottom_trans = 0;
		int red = 100;
		int green = 100;
		int blue = 100;
		int saturation = 100;
		int effect_mode = 0;
		int effect_power = 0;
		// Extensions
		bool flip_x = false;
		bool flip_y = false;
		int blend_mode = 0;
		int origin = 0;
	};
	struct ShowParams : Params {
		std::string name;
		// RPG Maker 2k3 1.12
		int spritesheet_cols = 1;
		int spritesheet_rows = 1;
		int spritesheet_frame = 0;
		int spritesheet_speed = 0;
		int map_layer = 7;
		int battle_layer = 0;
		// erase_on_map_change | affected_by_flash | affected_by_shake
		int flags = 1 | 32 | 64;
		bool spritesheet_play_once = false;
		bool use_transparent_color = false;
		bool fixed_to_map = false;
	};

	struct MoveParams : Params {
		int duration = 0;
	};

	bool Show(int id, const ShowParams& params);
	void Move(int id, const MoveParams& params);
	void Erase(int id);
	void EraseAll();

	void Update(bool is_battle);

	void OnMapChange();
	void OnBattleEnd();
	void OnMapScrolled(int dx, int dy);

	struct Picture {
		Picture(int id) { data.ID = id; }
		Picture(lcf::rpg::SavePicture data);

		std::unique_ptr<Sprite_Picture> sprite;
		lcf::rpg::SavePicture data;
		FileRequestBinding request_id;
		bool needs_update = false;
		int origin = 0;

		void Update(bool is_battle);

		bool IsOnMap() const;
		bool IsOnBattle() const;
		int NumSpriteSheetFrames() const;

		ShowParams GetShowParams() const;
		void SetNonEffectParams(const Params& params, bool set_positions);

		bool Show(const ShowParams& params);
		void Move(const MoveParams& params);
		void Erase();
		bool Exists() const;

		void CreateSprite();

		bool IsRequestPending() const;
		void MakeRequestImportant() const;

		void OnPictureSpriteReady();
		void ApplyOrigin(bool is_move);
		void OnMapScrolled(int dx, int dy);

		void AttachWindow(const Window_Base& window);
		bool IsWindowAttached() const;
	};

	Picture& GetPicture(int id);
	Picture* GetPicturePtr(int id);

private:
	void RequestPictureSprite(Picture& pic);
	void OnPictureSpriteReady(FileRequestResult*, int id);

	std::vector<Picture> pictures;
	int frame_counter = 0;
};

inline bool Game_Pictures::Picture::IsOnMap() const {
	return data.map_layer > 0;
}

inline bool Game_Pictures::Picture::IsOnBattle() const {
	return data.battle_layer > 0;
}

#endif
