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
#include "game_picture.h"
#include "sprite.h"
#include <vector>

////////////////////////////////////////////////////////////
/// Picture class
////////////////////////////////////////////////////////////

Picture::PictureState::PictureState() :
	red(255.0),
	green(255.0),
	blue(255.0),
	saturation(255.0),
	magnify(100.0),
	top_trans(0.0),
	bottom_trans(0.0)
{}

Picture::Picture() :
	shown(false),
	scrolls(false),
	rotate(false),
	waver(false),
	speed(0),
	value(0.0),
	sprite(NULL)
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
	if (!shown)
		return;

	PictureState& st = current_state;

	sprite->SetX((int)st.x);
	sprite->SetY((int)st.y);
	sprite->SetZ(9999);
	sprite->SetZoomX(st.magnify / 100.0);
	sprite->SetZoomY(st.magnify / 100.0);
	sprite->SetOx((int)(sprite->GetBitmap()->GetWidth() * st.magnify / 200.0));
	sprite->SetOy((int)(sprite->GetBitmap()->GetHeight() * st.magnify / 200.0));
	sprite->SetAngle(rotate ? value : 0.0);
	sprite->SetOpacity((int)(255 * (100 - st.top_trans) / 100));
	// TODO: bottom_trans
	sprite->SetTone(Tone((int) ((st.red        - 100) * 255 / 100),
						 (int) ((st.green      - 100) * 255 / 100),
						 (int) ((st.blue       - 100) * 255 / 100),
						 (int) ((st.saturation - 100) * 255 / 100)));
}

void Picture::Show(const std::string& _name) {
	name = _name;
	shown = true;
	duration = 0;

	if (sprite != NULL) {
		delete sprite;
		sprite = NULL;
	}

	Bitmap* bitmap = Cache::Picture(name);
	sprite = new Sprite();
	sprite->SetBitmap(bitmap);
	sprite->SetOx(bitmap->GetWidth() / 2);
	sprite->SetOy(bitmap->GetHeight() / 2);
}

void Picture::Erase() {
	shown = false;
	if (sprite != NULL)
		delete sprite;
	sprite = NULL;
}

void Picture::UseTransparent(bool flag) {
	use_trans = flag;
}

void Picture::Scrolls(bool flag) {
	scrolls = flag;
}

void Picture::Move(int x, int y) {
	PictureState& st = finish_state;
	st.x = x;
	st.y = y;
}

void Picture::Color(int r, int g, int b, int s) {
	PictureState& st = finish_state;
	st.red = r;
	st.green = g;
	st.blue = b;
	st.saturation = s;
}

void Picture::Magnify(int scale) {
	PictureState& st = finish_state;
	st.magnify = scale;
}

void Picture::Transparency(int t, int b) {
	PictureState& st = finish_state;
	st.top_trans = t;
	st.bottom_trans = b;
}

void Picture::Rotate(int _speed) {
	rotate = true;
	waver = false;
	speed = _speed;
	value = 0.0;
}

void Picture::Waver(int _speed) {
	rotate = false;
	waver = true;
	speed = _speed;
	value = 0.0;
}

void Picture::StopEffects() {
	rotate = false;
	waver = false;
}

void Picture::Transition(int tenths) {
	duration = tenths * DEFAULT_FPS / 10;
	if (tenths == 0) {
		current_state = finish_state;
		UpdateSprite();
	}
}

static double interpolate(double d, double x0, double x1) {
	return (x0 * (d - 1) + x1) / d;
}

void Picture::Update() {
	if (!shown)
		return;

	if (rotate || waver)
		value += speed;

	if (duration > 0) {
		PictureState& st = current_state;
		PictureState& st1 = finish_state;
		double k = duration;

		st.x = interpolate(k, st.x, st1.x);
		st.y = interpolate(k, st.y, st1.y);
		st.red = interpolate(k, st.red, st1.red);
		st.green = interpolate(k, st.green, st1.green);
		st.blue = interpolate(k, st.blue, st1.blue);
		st.saturation = interpolate(k, st.saturation, st1.saturation);
		st.magnify = interpolate(k, st.magnify, st1.magnify);
		st.top_trans = interpolate(k, st.top_trans, st1.top_trans);
		st.bottom_trans = interpolate(k, st.bottom_trans, st1.bottom_trans);

		duration--;
	}

	UpdateSprite();
}

