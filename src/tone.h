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

#ifndef EP_TONE_H
#define EP_TONE_H

/**
 * Tone class.
 */
class Tone {
public:
	/**
	 * Constructor. All values are set to 0.
	 */
	Tone();

	/**
	 * Constructor.
	 *
	 * @param red red component.
	 * @param green green component.
	 * @param blue blue component.
	 * @param gray gray component.
	 */
	Tone(int red, int green, int blue, int gray);

	/**
	 * Set all color properties.
	 *
	 * @param red red component.
	 * @param green green component.
	 * @param blue blue component.
	 * @param gray gray component.
	 */
	void Set(int red, int green, int blue, int gray);

	/** Red component. */
	int red;

	/** Green component. */
	int green;

	/** Blue component. */
	int blue;

	/** Gray component. */
	int gray;
};

inline Tone Blend(const Tone& l, const Tone& r) {
	return Tone((l.red + r.red) / 2,
			(l.green + r.green) / 2,
			(l.blue + r.blue) / 2,
			(l.gray * r.gray) / 128);
}

inline bool operator==(const Tone &l, const Tone& r) {
	return l.red == r.red
		&& l.green == r.green
		&& l.blue == r.blue
		&& l.gray == r.gray;
}

inline bool operator!=(const Tone &l, const Tone& r) {
	return !(l == r);
}


#endif
