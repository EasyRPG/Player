/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "options.h"
#include "cache.h"
#include "main_data.h"
#include "sprite.h"
#include "game_picture.h"
#include <vector>

////////////////////////////////////////////////////////////
/// Picture class
////////////////////////////////////////////////////////////

Picture::Picture(int ID) :
	data(Main_Data::game_data.pictures[ID - 1]), sprite(NULL)
{
	Transition(0);
}

Picture::~Picture()
{
	if (sprite != NULL)
		delete sprite;
	sprite = NULL;
}

void Picture::UpdateSprite() {
	if (sprite == NULL)
		return;
	if (data.name.empty())
		return;

	sprite->SetX((int) data.current_x);
	sprite->SetY((int) data.current_y);
	sprite->SetZ(1000 + data.ID);
	sprite->SetZoomX(data.current_magnify / 100.0);
	sprite->SetZoomY(data.current_magnify / 100.0);
	sprite->SetOx((int)(sprite->GetBitmap()->GetWidth() * data.current_magnify / 200.0));
	sprite->SetOy((int)(sprite->GetBitmap()->GetHeight() * data.current_magnify / 200.0));
	sprite->SetAngle(data.effect_mode == 1 ? data.current_rotation : 0.0);
	sprite->SetWaverPhase(data.effect_mode == 2 ? data.current_waver : 0.0);
	sprite->SetWaverDepth(data.effect_mode == 2 ? data.effect2_speed : 0);
	sprite->SetOpacity(
		(int)(255 * (100 - data.current_top_trans) / 100),
		(int)(255 * (100 - data.current_bot_trans) / 100));
	if (data.current_bot_trans != data.current_top_trans)
		sprite->SetBushDepth(sprite->GetHeight() / 2);
	sprite->SetTone(Tone((int) ((data.current_red        - 100) * 255 / 100),
						 (int) ((data.current_green      - 100) * 255 / 100),
						 (int) ((data.current_blue       - 100) * 255 / 100),
						 (int) ((100 - data.current_sat) * 255 / 100)));
}

void Picture::Show(const std::string& _name) {
	data.name = _name;
	data.time_left = 0;

	if (sprite != NULL) {
		delete sprite;
		sprite = NULL;
	}

	Bitmap* bitmap = Cache::Picture(data.name);
	sprite = new Sprite();
	sprite->SetBitmap(bitmap);
	sprite->SetOx(bitmap->GetWidth() / 2);
	sprite->SetOy(bitmap->GetHeight() / 2);
}

void Picture::Erase() {
	data.name.clear();
	if (sprite != NULL)
		delete sprite;
	sprite = NULL;
}

void Picture::UseTransparent(bool flag) {
	data.transparency = flag;
}

void Picture::Scrolls(bool flag) {
	data.picture_scrolls = flag;
}

void Picture::Move(int x, int y) {
	data.finish_x = x;
	data.finish_y = y;
}

void Picture::Color(int r, int g, int b, int s) {
	data.finish_red = r;
	data.finish_green = g;
	data.finish_blue = b;
	data.finish_sat = s;
}

void Picture::Magnify(int scale) {
	data.finish_magnify = scale;
}

void Picture::Transparency(int t, int b) {
	data.finish_top_trans = t;
	data.finish_bot_trans = b;
}

void Picture::Rotate(int speed) {
	data.effect_mode = 1;
	data.effect_speed = data.effect2_speed = speed;
	data.current_rotation = 0;
}

void Picture::Waver(int depth) {
	data.effect_mode = 2;
	data.effect_speed = data.effect2_speed = depth;
	data.current_waver = 0;
}

void Picture::StopEffects() {
	data.effect_mode = 0;
}

void Picture::Transition(int tenths) {
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
		UpdateSprite();
	}
}

static double interpolate(double d, double x0, double x1) {
	return (x0 * (d - 1) + x1) / d;
}

void Picture::Update() {
	if (data.name.empty())
		return;

	if (data.effect_mode == 1)
		data.current_rotation += data.effect_speed;
	if (data.effect_mode == 2)
		data.current_waver += data.effect2_speed;

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

		data.time_left--;
	}

	UpdateSprite();
}

