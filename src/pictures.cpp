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

#include "pictures.h"
#include <vector>

////////////////////////////////////////////////////////////
/// Picture class
////////////////////////////////////////////////////////////

static const int FramesPerSec = 60;

static std::vector<Picture> pictures;

Picture& Pictures::Get(int id) {
	if ((size_t) id >= pictures.size())
		pictures.resize(id);
	return pictures[id - 1];
}

static int Frames() {
	return 0;	// TODO
}

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
	value(0.0)
{
	Transition(0);
}

Picture::~Picture()
{
}

void Picture::Show(const std::string& _name) {
	name = _name;
	shown = true;
	start_time = Frames();
	finish_time = start_time;
}

void Picture::Erase() {
	shown = false;
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

void Picture::Set() {
	start_state = finish_state;
	current_state = finish_state;
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
	current_state = start_state;
	start_time = Frames();
	finish_time = start_time + tenths * FramesPerSec / 10;
	transition = tenths > 0;
}

static double interpolate(double k, double x0, double x1) {
	return x0 + k * (x1 - x0);
}

void Picture::Update() {
	if (finish_time <= start_time)
		return;

	int t = Frames();
	if (rotate || waver) {
		double dt = t - start_time;
		value = dt * speed / FramesPerSec;
	}

	if (!transition)
		return;

	PictureState& st0 = start_state;
	PictureState& st1 = finish_state;
	PictureState& st = current_state;

	double k = (double) (t - start_time) / (finish_time - start_time);
	if (k > 1.0)
		k = 1.0;

	st.x = interpolate(k, st0.x, st1.x);
	st.y = interpolate(k, st0.y, st1.y);
	st.red = interpolate(k, st0.red, st1.red);
	st.green = interpolate(k, st0.green, st1.green);
	st.blue = interpolate(k, st0.blue, st1.blue);
	st.saturation = interpolate(k, st0.saturation, st1.saturation);
	st.magnify = interpolate(k, st0.magnify, st1.magnify);
	st.top_trans = interpolate(k, st0.top_trans, st1.top_trans);
	st.bottom_trans = interpolate(k, st0.bottom_trans, st1.bottom_trans);

	if (t >= finish_time)
		transition = false;
}

