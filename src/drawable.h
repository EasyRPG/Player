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
	TypeDefault
};

enum Priority {
	Priority_Background = 5 << 24,
	Priority_TilesetBelow = 10 << 24,
	Priority_EventsBelow = 15 << 24,
	Priority_Player = 20 << 24, // In Map, shared with "same as hero" events
	Priority_Battler = 20 << 24, // In Battle (includes animations)
	Priority_TilesetAbove = 25 << 24,
	Priority_EventsAbove = 30 << 24,
	Priority_Weather = 35 << 24,
	Priority_Screen = 40 << 24,
	Priority_PictureNew = 45 << 24, // Pictures in RPG2k Value! and RPG2k3 >=1.05, shared
	Priority_BattleAnimation = 50 << 24,
	Priority_PictureOld = 55 << 24, // Picture in RPG2k <1.5 and RPG2k3 <1.05, shared
	Priority_Window = 60 << 24,
	Priority_Timer = 65 << 24,
	Priority_Frame = 70 << 24,
	Priority_Overlay = 75 << 24
};

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
