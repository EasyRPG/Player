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
#include <cmath>

#include "sprite_picture.h"
#include "main_data.h"
#include "game_pictures.h"
#include "game_battle.h"
#include "game_screen.h"
#include "game_windows.h"
#include "player.h"
#include "bitmap.h"
#include "game_map.h"
#include "game_variables.h"

Sprite_Picture::Sprite_Picture(int pic_id, Drawable::Flags flags)
	: Sprite(flags),
	pic_id(pic_id),
	feature_spritesheet(Player::IsRPG2k3ECommands()),
	feature_priority_layers(Player::IsMajorUpdatedVersion()),
	feature_bottom_trans(Player::IsRPG2k3() && !Player::IsRPG2k3E() && !Player::game_config.patch_powermode.Get())
{
	// Initialize Z value for legacy pictures. Will be overriden in OnPictureShow if
	// priority layers feature is enabled.
	// Battle Animations are below pictures
	SetZ(Priority_PictureOld + pic_id);
}

void Sprite_Picture::OnPictureShow() {
	last_spritesheet_frame = -1;

	const bool is_battle = Game_Battle::IsBattleRunning();
	const auto& pic = Main_Data::game_pictures->GetPicture(pic_id);

	if (is_battle && Player::game_config.patch_pics_in_battle.Get() > 0 && pic.data.battle_layer == 0) {
		UpdateBattleZ();
		return;
	}

	if (feature_priority_layers) {
		// Battle Animations are above pictures
		Drawable::Z_t priority;
		if (is_battle) {
			priority = Drawable::GetPriorityForBattleLayer(pic.data.battle_layer);
		} else {
			priority = Drawable::GetPriorityForMapLayer(pic.data.map_layer);
		}
		if (priority > 0) {
			// Small offset (10) to ensure there is space for graphics that are
			// drawn at the image position (e.g. DynRPG Text Plugin)
			SetZ(priority + pic_id * 10);
		}
	}
}

void Sprite_Picture::UpdateBattleZ() {
	const int pib_var = Player::game_config.patch_pics_in_battle.Get();
	if (pib_var <= 0) {
		return;
	}

	int layer = Main_Data::game_variables->Get(pib_var);
	Drawable::Z_t priority = 0;
	switch (layer) {
		case 0:
			// Above Backdrop, below Heroes/Monsters
			priority = Drawable::GetPriorityForBattleLayer(lcf::rpg::SavePicture::BattleLayer_background);
			break;
		case 1:
			// Above Battle Animations, below Weather
			priority = Priority_BattleAnimation;
			break;
		case 2:
			// Above Windows/Status, below Timers
			priority = Drawable::GetPriorityForBattleLayer(lcf::rpg::SavePicture::BattleLayer_windows_and_status);
			break;
		default:
			return;
	}

	if (priority > 0) {
		SetZ(priority + pic_id * 10);
	}
}

void Sprite_Picture::UpdateMapZ() {
	const auto& pic = Main_Data::game_pictures->GetPicture(pic_id);

	if (feature_priority_layers) {
		Drawable::Z_t priority = Drawable::GetPriorityForMapLayer(pic.data.map_layer);
		if (priority > 0) {
			SetZ(priority + pic_id * 10);
			return;
		}
	}

	SetZ(Priority_PictureOld + pic_id);
}

bool Sprite_Picture::RefreshPictureState() {
	const auto& pic = Main_Data::game_pictures->GetPicture(pic_id);
	const auto& data = pic.data;

	auto& bitmap = GetBitmap();

	if (!bitmap) {
		return false;
	}

	if (pic.IsWindowAttached()) {
		// Paint the Window on the Picture
		const auto& window = Main_Data::game_windows->GetWindow(pic_id);
		window.window->Draw(*bitmap.get());
	}

	const bool is_battle = Game_Battle::IsBattleRunning();

	const int pib_var = Player::game_config.patch_pics_in_battle.Get();
	const bool pics_in_battle = (pib_var > 0);

	if (is_battle) {
		if (!pic.IsOnBattle()) {
			if (!pics_in_battle || !pic.IsOnMap()) {
				return false;
			}
			// PicsInBattle hides battle pictures if the layer variable is not 0, 1, or 2
			int battle_layer_var = Main_Data::game_variables->Get(pib_var);
			if (battle_layer_var < 0 || battle_layer_var > 2) {
				return false;
			}
		}
	} else if (!pic.IsOnMap()) {
		return false;
	}

	// RPG Maker 2k3 1.12: Spritesheets
	if (feature_spritesheet
			&& pic.NumSpriteSheetFrames() > 1
			&& last_spritesheet_frame != data.spritesheet_frame)
	{
		last_spritesheet_frame = data.spritesheet_frame;

		const int sw = bitmap->GetWidth() / data.spritesheet_cols;
		const int sh = bitmap->GetHeight() / data.spritesheet_rows;
		const int sx = sw * ((data.spritesheet_frame) % data.spritesheet_cols);
		const int sy = sh * ((data.spritesheet_frame) / data.spritesheet_cols % data.spritesheet_rows);

		SetSrcRect(Rect{ sx, sy, sw, sh });
	}

	int x = data.current_x;
	int y = data.current_y;
	if (data.flags.affected_by_shake) {
		x -= Main_Data::game_screen->GetShakeOffsetX();
		y -= Main_Data::game_screen->GetShakeOffsetY();
	}

	if (Player::game_config.fake_resolution.Get()) {
		if (data.fixed_to_map && !is_battle) {
			// If the picture scrolls with the map, apply the black border if the map is too small
			int offset_x = 0;
			int map_width = Game_Map::GetTilesX() * TILE_SIZE;
			if (map_width < Player::screen_width) {
				offset_x += (Player::screen_width - map_width) / 2;
			}
			SetX(x + offset_x);

			int offset_y = 0;
			int map_height = Game_Map::GetTilesY() * TILE_SIZE;
			if (map_height < Player::screen_height) {
				offset_y += (Player::screen_height - map_height) / 2;
			}
			SetY(y + offset_y);
		} else {
			// If the picture doesn't scroll with the map, simulate the 340x240 screen
			SetX(x + Player::menu_offset_x);
			SetY(y + Player::menu_offset_y);
		}
	} else {
		SetX(x);
		SetY(y);
	}

	SetZoomX(data.current_magnify / 100.0);
	SetZoomY(data.current_magnify / 100.0);
	if (Player::IsPatchManiac()) {
		SetZoomY(data.maniac_current_magnify_height / 100.0);
	}

	auto sr = GetSrcRect();
	SetOx(sr.width / 2);
	SetOy(sr.height / 2);

	if (data.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
		SetAngle(data.current_rotation * (2 * M_PI) / 360);
	} else if (data.effect_mode != lcf::rpg::SavePicture::Effect_wave) {
		SetAngle(data.current_rotation * (2 * M_PI) / 256);
	} else {
		SetAngle(0.0);
	}
	SetWaverPhase(data.effect_mode == lcf::rpg::SavePicture::Effect_wave ? data.current_waver * (2 * M_PI) / 256 : 0.0);
	SetWaverDepth(data.effect_mode == lcf::rpg::SavePicture::Effect_wave ? data.current_effect_power * 2 : 0);

	// Only older versions of RPG_RT apply the effects of current_bot_trans chunk.
	const auto top_trans = data.current_top_trans;
	const auto bottom_trans = feature_bottom_trans ? data.current_bot_trans : top_trans;

	SetOpacity(
		(int)(255 * (100 - top_trans) / 100),
		(int)(255 * (100 - bottom_trans) / 100));

	if (bottom_trans != top_trans) {
		SetBushDepth(GetHeight() / 2);
	} else {
		SetBushDepth(0);
	}

	auto tone = Tone((int) (data.current_red * 128 / 100),
			(int) (data.current_green * 128 / 100),
			(int) (data.current_blue * 128 / 100),
			(int) (data.current_sat * 128 / 100));
	if (data.flags.affected_by_tint) {
		auto screen_tone = Main_Data::game_screen->GetTone();
		tone = Blend(tone, screen_tone);
	}
	SetTone(tone);

	if (data.flags.affected_by_flash) {
		SetFlashEffect(Main_Data::game_screen->GetFlashColor());
	} else {
		SetFlashEffect(Color());
	}

	SetFlipX((data.easyrpg_flip & lcf::rpg::SavePicture::EasyRpgFlip_x) == lcf::rpg::SavePicture::EasyRpgFlip_x);
	SetFlipY((data.easyrpg_flip & lcf::rpg::SavePicture::EasyRpgFlip_y) == lcf::rpg::SavePicture::EasyRpgFlip_y);
	SetBlendType(data.easyrpg_blend_mode);

	// Don't draw anything if zoom is at zero, helps avoid a glitchy rotated sprite in the top left corner
	if (GetZoomX() <= 0.0 || GetZoomY() <= 0.0) {
		return false;
	}

	return true;
}

void Sprite_Picture::Draw(Bitmap& dst) {
	if (!RefreshPictureState()) {
		return;
	}

	Sprite::Draw(dst);
}

int Sprite_Picture::GetFrameWidth() const {
	const auto& pic = Main_Data::game_pictures->GetPicture(pic_id);
	const auto& data = pic.data;

	auto& bitmap = GetBitmap();
	assert(bitmap);

	if (feature_spritesheet && pic.NumSpriteSheetFrames() > 1) {
		return bitmap->GetWidth() / data.spritesheet_cols;
	} else {
		return bitmap->GetWidth();
	}
}

int Sprite_Picture::GetFrameHeight() const {
	const auto& pic = Main_Data::game_pictures->GetPicture(pic_id);
	const auto& data = pic.data;

	auto& bitmap = GetBitmap();
	assert(bitmap);

	if (feature_spritesheet && pic.NumSpriteSheetFrames() > 1) {
		return bitmap->GetHeight() / data.spritesheet_rows;
	} else {
		return bitmap->GetHeight();
	}
}
