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
	PriorityBackground = 10 << 24,
	PriorityPanorama = 15 << 24,
	PriorityMap = 20 << 24, // Map and Battle are two different scenes...
	PriorityBattle = 20 << 24, // ...will never conflict
	PriorityBattleUi = 25 << 24,
	PriorityAirshipShadow = 25 << 24,
	PriorityPictureNew = 30 << 24, // Pictures in RPG2k Value! and RPG2k3 >=1.05
	PriorityBattleAnimation = 35 << 24,
	PriorityPictureOld = 40 << 24, // Picture in RPG2k <1.5 and RPG2k3 <1.05
	PriorityWeather = 45 << 24,
	PriorityScreen = 50 << 24,
	PriorityTimer = 55 << 24,
	PriorityWindow = 60 << 24,
	PriorityFrame = 65 << 24,
	PriorityMessageOverlay = 70 << 24
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
