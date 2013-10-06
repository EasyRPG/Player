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

#ifndef _EASYRPG_GRAPHIC_WEATHER_H_
#define _EASYRPG_GRAPHIC_WEATHER_H_

// Headers
#include <string>
#include "drawable.h"
#include "system.h"
#include "zobj.h"

class Bitmap;
class BitmapScreen;

/**
 * Renders the weather effects.
 */
class Weather : public Drawable {
public:
	Weather();
	~Weather();

	void Draw(int z_order);
	void Update();

	unsigned long GetId() const;
	int GetZ() const;
	DrawableType GetType() const;

private:
	void DrawRain();
	void DrawSnow();
	void DrawFog();
	void DrawSandstorm();

	static const int z = 1001;
	static const DrawableType type = TypeWeather;

	unsigned long ID;
	ZObj* zobj;

	BitmapScreenRef weather_screen;
	//boost::scoped_ptr<Plane> weather_plane;
	BitmapRef weather_surface;
	BitmapRef snow_bitmap;
	BitmapRef rain_bitmap;

	bool dirty;
};

#endif
