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

/**
 * Drawable virtual
 */
class Drawable {
public:
	using Z_t = uint64_t;

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

	Drawable(Z_t z, Flags flags = Flags::Default);

	Drawable(const Drawable&) = delete;
	Drawable& operator=(const Drawable&) = delete;

	virtual ~Drawable();

	virtual void Draw(Bitmap& dst) = 0;

	Z_t GetZ() const;

	void SetZ(Z_t z);

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
	static Z_t GetPriorityForMapLayer(int which);

	/**
	 * Converts a RPG Maker battle layer value into a EasyRPG priority value.
	 *
	 * @return Priority or 0 when not found
	 */
	static Z_t GetPriorityForBattleLayer(int which);
private:
	Z_t _z = 0;
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

inline Drawable::Drawable(Z_t z, Flags flags)
	: _z(z),
	_flags(flags)
{
}

inline Drawable::Z_t Drawable::GetZ() const {
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

// Upper 8 bit are reserved for the layer 
static constexpr uint64_t z_offset = 64 - 8;

// Lower 56 Bit are free to use
// Keep a gap of 1 between layers everywhere because Pictures on the same layer have a z_offset of + 1
enum Priority : Drawable::Z_t {
	Priority_Background = 10ULL << z_offset,
	Priority_TilesetBelow = 20ULL << z_offset,
	Priority_EventsBelow = 30ULL << z_offset,
	Priority_Player = 40ULL << z_offset, // In Map, shared with "same as hero" events
	Priority_Battler = 40ULL << z_offset, // In Battle (includes animations)
	Priority_TilesetAbove = 50ULL << z_offset,
	Priority_EventsAbove = 60ULL << z_offset,
	Priority_EventsFlying = 70ULL << z_offset,
	Priority_Weather = 80ULL << z_offset,
	Priority_Screen = 90ULL << z_offset,
	Priority_PictureNew = 100ULL << z_offset, // Pictures in RPG2k Value! and RPG2k3 >=1.05, shared
	Priority_BattleAnimation = 110ULL << z_offset,
	Priority_PictureOld = 120ULL << z_offset, // Picture in RPG2k <1.5 and RPG2k3 <1.05, shared
	Priority_Window = 130ULL << z_offset,
	Priority_Timer = 140ULL << z_offset,
	Priority_Frame = 150ULL << z_offset,
	Priority_Transition = 160ULL << z_offset,
	Priority_Overlay = 170ULL << z_offset,
	Priority_Maximum = 255ULL << z_offset // Higher values will overflow
};

#endif
