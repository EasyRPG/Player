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

#ifndef _PICTURE_H_
#define _PICTURE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include <string>
#include <vector>

////////////////////////////////////////////////////////////
/// Picture class
////////////////////////////////////////////////////////////

class Sprite;

class Picture {
public:
	class PictureState {
		PictureState();

		double x;
		double y;
		double red;
		double green;
		double blue;
		double saturation;
		double magnify;
		double top_trans;
		double bottom_trans;

		friend class Picture;
	};
	Picture();
	~Picture();

	void Show(const std::string& name);
	void Erase();
	void UseTransparent(bool flag);
	void Scrolls(bool flag);
	void Move(int x, int y);
	void Color(int r, int g, int b, int s);
	void Magnify(int scale);
	void Transparency(int t, int b);
	void Rotate(int speed);
	void Waver(int depth);
	void StopEffects();
	void Transition(int tenths);

	void Update();

private:
	std::string name;
	PictureState finish_state;
	PictureState current_state;
	int duration;
	bool shown;
	bool scrolls;
	bool use_trans;
	bool rotate;
	bool waver;
	int speed;
	int depth;
	double value;

	static const int waver_speed = 10;

	Sprite* sprite;

	void UpdateSprite();
};

#endif

