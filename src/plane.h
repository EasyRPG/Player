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

#ifndef EP_PLANE_H
#define EP_PLANE_H

// Headers
#include "system.h"
#include "color.h"
#include "drawable.h"
#include "tone.h"

/**
 * Plane class.
 */
class Plane : public Drawable {
public:
	Plane();

	void Draw(Bitmap& dst) override;

	BitmapRef const& GetBitmap() const;
	void SetBitmap(BitmapRef const& bitmap);
	int GetOx() const;
	void SetOx(int ox);
	int GetOy() const;
	void SetOy(int oy);
	Tone GetTone() const;
	void SetTone(Tone tone);

private:
	BitmapRef bitmap;
	BitmapRef tone_bitmap;

	Tone tone_effect;

	int ox = 0;
	int oy = 0;
	bool needs_refresh = false;
};

inline BitmapRef const& Plane::GetBitmap() const {
	return bitmap;
}

inline void Plane::SetBitmap(BitmapRef const& nbitmap) {
	bitmap = nbitmap;

	needs_refresh = true;
}

inline int Plane::GetOx() const {
	return ox;
}

inline void Plane::SetOx(int nox) {
	ox = nox;
}

inline int Plane::GetOy() const {
	return oy;
}

inline void Plane::SetOy(int noy) {
	oy = noy;
}

inline Tone Plane::GetTone() const {
	return tone_effect;
}

inline void Plane::SetTone(Tone tone) {
	if (tone_effect != tone) {
		tone_effect = tone;
		needs_refresh = true;
	}
}

#endif
