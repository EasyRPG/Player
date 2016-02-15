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

// Headers
#include "bitmap.h"
#include "async_handler.h"
#include "options.h"
#include "cache.h"
#include "game_map.h"
#include "game_picture.h"
#include "player.h"
#include "main_data.h"

/**
 * Picture class.
 */
Game_Picture::Game_Picture(int ID) :
	id(ID),
	old_map_x(0),
	old_map_y(0),
	request(NULL)
{
	SetTransition(0);
}

Game_Picture::~Game_Picture() {
	GetData().name = "";
}

void Game_Picture::UpdateSprite() {
	RPG::SavePicture& data = GetData();

	if (!sprite)
		return;
	if (data.name.empty())
		return;

	sprite->SetX((int)data.current_x);
	sprite->SetY((int)data.current_y);
	sprite->SetZ(1100 + data.ID);
	sprite->SetZoomX(data.current_magnify / 100.0);
	sprite->SetZoomY(data.current_magnify / 100.0);
	sprite->SetOx((int)(sprite->GetBitmap()->GetWidth() / 2));
	sprite->SetOy((int)(sprite->GetBitmap()->GetHeight() / 2));

	sprite->SetAngle(data.effect_mode == 1 ? data.current_rotation * 360 / 256 : 0.0);
	sprite->SetWaverPhase(data.effect_mode == 2 ? data.current_waver : 0.0);
	sprite->SetWaverDepth(data.effect_mode == 2 ? data.current_effect * 2 : 0);
	sprite->SetOpacity(
		(int)(255 * (100 - data.current_top_trans) / 100),
		(int)(255 * (100 - data.current_bot_trans) / 100));
	if (data.current_bot_trans != data.current_top_trans)
		sprite->SetBushDepth(sprite->GetHeight() / 2);
	sprite->SetTone(Tone((int) (data.current_red * 128 / 100),
						 (int) (data.current_green * 128 / 100),
						 (int) (data.current_blue * 128 / 100),
						 (int) (data.current_sat * 128 / 100)));
}

void Game_Picture::Show(const std::string& _name, bool _transparency) {
	RPG::SavePicture& data = GetData();

	data.name = _name;
	data.transparency = _transparency;
	data.time_left = 0;

	if (request) {
		request->Unbind(request_id);
	}
	request = AsyncHandler::RequestFile("Picture", data.name);
	request_id = request->Bind(&Game_Picture::OnPictureSpriteReady, this);
	request->Start();

	old_map_x = Game_Map::GetDisplayX();
	old_map_y = Game_Map::GetDisplayY();
}

void Game_Picture::OnPictureSpriteReady(FileRequestResult*) {
	RPG::SavePicture& data = GetData();

	BitmapRef bitmap = Cache::Picture(data.name, data.transparency);

	sprite.reset(new Sprite());
	sprite->SetBitmap(bitmap);
	UpdateSprite();
}

void Game_Picture::Erase() {
	RPG::SavePicture& data = GetData();

	data.name.clear();
	sprite.reset();
}

void Game_Picture::SetFixedToMap(bool flag) {
	RPG::SavePicture& data = GetData();

	data.fixed_to_map = flag;
}

void Game_Picture::SetMovementEffect(int x, int y) {
	RPG::SavePicture& data = GetData();

	data.finish_x = x;
	data.finish_y = y;
}

void Game_Picture::SetColorEffect(int r, int g, int b, int s) {
	RPG::SavePicture& data = GetData();

	data.finish_red = r;
	data.finish_green = g;
	data.finish_blue = b;
	data.finish_sat = s;
}

void Game_Picture::SetZoomEffect(int scale) {
	RPG::SavePicture& data = GetData();

	data.finish_magnify = scale;
}

void Game_Picture::SetTransparencyEffect(int top, int bottom) {
	RPG::SavePicture& data = GetData();

	data.finish_top_trans = top;
	data.finish_bot_trans = bottom;
}

void Game_Picture::SetRotationEffect(int speed) {
	RPG::SavePicture& data = GetData();

	if (!data.time_left || Player::IsRPG2k3()) {
		if (data.effect_mode != 1)
			data.current_rotation = 0;
		data.effect_mode = 1;
	}
	data.finish_effect = speed;
}

void Game_Picture::SetWaverEffect(int depth) {
	RPG::SavePicture& data = GetData();

	if (!data.time_left || Player::IsRPG2k3()) {
		if (data.effect_mode != 2)
			data.current_waver = 0;
		data.effect_mode = 2;
	}
	data.finish_effect = depth;
}

void Game_Picture::StopEffects() {
	RPG::SavePicture& data = GetData();

	if (!data.time_left || Player::IsRPG2k3())
		data.effect_mode = 0;
}

void Game_Picture::SetTransition(int tenths) {
	RPG::SavePicture& data = GetData();

	data.time_left = tenths * DEFAULT_FPS / 10;

	if (tenths == 0) {
		data.current_x			= data.finish_x;
		data.current_y			= data.finish_y;
		data.current_red		= data.finish_red;
		data.current_green		= data.finish_green;
		data.current_blue		= data.finish_blue;
		data.current_sat		= data.finish_sat;
		data.current_magnify	= data.finish_magnify;
		data.current_top_trans	= data.finish_top_trans;
		data.current_bot_trans	= data.finish_bot_trans;
		data.current_effect		= data.finish_effect;
		UpdateSprite();
	}
}

static double interpolate(double d, double x0, double x1) {
	return (x0 * (d - 1) + x1) / d;
}

void Game_Picture::Update() {
	RPG::SavePicture& data = GetData();

	if (data.name.empty())
		return;

	if (data.fixed_to_map) {
		// Instead of modifying the Ox/Oy offset the real position is altered
		// based on map scroll because of savegame compatibility with RPG_RT

		if (old_map_x != Game_Map::GetDisplayX()) {
			int mx = (old_map_x - Game_Map::GetDisplayX()) / TILE_SIZE;

			data.finish_x += mx;
			data.current_x += mx;
		}
		if (old_map_y != Game_Map::GetDisplayY()) {
			int my = (old_map_y - Game_Map::GetDisplayY()) / TILE_SIZE;

			data.finish_y += my;
			data.current_y += my;
		}

		old_map_x = Game_Map::GetDisplayX();
		old_map_y = Game_Map::GetDisplayY();
	}

	if (data.effect_mode == 1)
		data.current_rotation += data.current_effect;
	if (data.effect_mode == 2)
		data.current_waver += 10;

	if (data.time_left > 0) {
		double k = data.time_left;

		data.current_x			= interpolate(k, data.current_x,			data.finish_x);
		data.current_y			= interpolate(k, data.current_y,			data.finish_y);
		data.current_red		= interpolate(k, data.current_red,			data.finish_red);
		data.current_green		= interpolate(k, data.current_green,		data.finish_green);
		data.current_blue		= interpolate(k, data.current_blue,			data.finish_blue);
		data.current_sat		= interpolate(k, data.current_sat,			data.finish_sat);
		data.current_magnify	= interpolate(k, data.current_magnify,		data.finish_magnify);
		data.current_top_trans	= interpolate(k, data.current_top_trans,	data.finish_top_trans);
		data.current_bot_trans	= interpolate(k, data.current_bot_trans,	data.finish_bot_trans);
		data.current_effect		= interpolate(k, data.current_effect,		data.finish_effect);

		data.time_left--;
	}

	UpdateSprite();
}

RPG::SavePicture& Game_Picture::GetData() {
	return Main_Data::game_data.pictures[id - 1];
}
