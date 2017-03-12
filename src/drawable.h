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

#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_

// What kind of drawable is the current one?
enum DrawableType {
	TypeWindow,
	TypeTilemap,
	TypeSprite,
	TypePlane,
	TypeBackground,
	TypeScreen,
	TypeFrame,
	TypeWeather,
	TypeOverlay,
	TypeDefault};

/**
 * Drawable virtual
 */
class Drawable {
public:
	virtual ~Drawable() {};

	virtual void Draw() = 0;

	virtual int GetZ() const = 0;

	virtual DrawableType GetType() const = 0;

	virtual bool IsGlobal() const { return false; }
};

#endif
