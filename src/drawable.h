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
	/** Flags with dictate certain attributes of drawables */
	enum class Flags : uint32_t {
		/** No flags */
		None = 0,
		/** This is a global drawable which will appear in all scenes */
		Global = 1,
		/** This is a shared drawable which will appear in all scenes that use shared drawables */
		Shared = 2,
		/** This flag indicates the drawable should not be drawn */
		Invisible = 4,
		/** The default flag set */
		Default = None
	};

	Drawable(int z, Flags flags = Flags::Default);

	Drawable(const Drawable&) = delete;
	Drawable& operator=(const Drawable&) = delete;

	virtual ~Drawable();

	virtual void Draw(Bitmap& dst) = 0;

	int GetZ() const;

	void SetZ(int z);

	/* @return true if this drawable should appear in all scenes */
	bool IsGlobal() const;

	/* @return true if this drawable should appear in all scenes that use shared drawables */
	bool IsShared() const;

	/* @return true if the drawable is currently visible */
	bool IsVisible() const;

	/**
	 * Set if the drawable should be visisble
	 *
	 * @param value whether is visible or not.
	 */
	void SetVisible(bool value);

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
	int32_t _z = 0;
	Flags _flags = Flags::Default;
};

inline Drawable::Flags operator|(Drawable::Flags l, Drawable::Flags r) {
	return static_cast<Drawable::Flags>(static_cast<unsigned>(l) | static_cast<unsigned>(r));
}

inline Drawable::Flags operator&(Drawable::Flags l, Drawable::Flags r) {
	return static_cast<Drawable::Flags>(static_cast<unsigned>(l) & static_cast<unsigned>(r));
}

inline Drawable::Flags operator^(Drawable::Flags l, Drawable::Flags r) {
	return static_cast<Drawable::Flags>(static_cast<unsigned>(l) ^ static_cast<unsigned>(r));
}

inline Drawable::Flags operator~(Drawable::Flags f) {
	return static_cast<Drawable::Flags>(~static_cast<unsigned>(f));
}

inline Drawable::Drawable(int z, Flags flags)
	: _z(z),
	_flags(flags)
{
}

inline int Drawable::GetZ() const {
	return _z;
}

inline bool Drawable::IsGlobal() const {
	return static_cast<bool>(_flags & Flags::Global);
}

inline bool Drawable::IsShared() const {
	return static_cast<bool>(_flags & Flags::Shared);
}

inline bool Drawable::IsVisible() const {
	return !static_cast<bool>(_flags & Flags::Invisible);
}

inline void Drawable::SetVisible(bool value) {
	_flags = value ? _flags & ~Flags::Invisible : _flags | Flags::Invisible;
}

#endif
