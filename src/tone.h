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

#ifndef _TONE_H_
#define _TONE_H_

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
	 * Equality operator.
	 */
	bool operator==(const Tone &other) const;

	/**
	 * Inequality operator.
	 */
	bool operator!=(const Tone &other) const;

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

#endif
