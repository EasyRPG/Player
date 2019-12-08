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

#ifndef EP_DRAWABLE_H
#define EP_DRAWABLE_H

#include <cstdint>
#include <memory>

class Bitmap;
class Drawable;

template <typename T>
static constexpr bool IsDrawable = std::is_base_of<Drawable,T>::value;

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
	TypeTransition,
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
	Priority_EventsFlying = 35 << 24,
	Priority_Weather = 36 << 24,
	Priority_Screen = 40 << 24,
	Priority_PictureNew = 45 << 24, // Pictures in RPG2k Value! and RPG2k3 >=1.05, shared
	Priority_BattleAnimation = 50 << 24,
	Priority_PictureOld = 55 << 24, // Picture in RPG2k <1.5 and RPG2k3 <1.05, shared
	Priority_Window = 60 << 24,
	Priority_Timer = 65 << 24,
	Priority_Frame = 70 << 24,
	Priority_Transition = 75 << 24,
	Priority_Overlay = 80 << 24,
	Priority_Maximum = 100 << 24
};

/**
 * Drawable virtual
 */
class Drawable {
public:
	Drawable(DrawableType type, int z, bool is_global);

	Drawable(const Drawable&) = delete;
	Drawable& operator=(const Drawable&) = delete;

	virtual ~Drawable();

	virtual void Draw(Bitmap& dst) = 0;

	int GetZ() const;

	void SetZ(int z);

	DrawableType GetType() const;

	bool IsGlobal() const;

	/**
	 * Converts a RPG Maker map layer value into a EasyRPG priority value.
	 *
	 * @return Priority or 0 when not found
	 */
	static int GetPriorityForMapLayer(int which);

	/**
	 * Converts a RPG Maker battle layer value into a EasyRPG priority value.
	 *
	 * @return Priority or 0 when not found
	 */
	static int GetPriorityForBattleLayer(int which);
private:
	int _z = 0;
	uint16_t _type = TypeDefault;
	bool _is_global = false;
};

inline int Drawable::GetZ() const {
	return _z;
}

inline DrawableType Drawable::GetType() const {
	return static_cast<DrawableType>(_type);
}

inline bool Drawable::IsGlobal() const {
	return _is_global;
}


#endif
