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

#ifndef EP_TELEPORT_TARGET_H
#define EP_TELEPORT_TARGET_H

#include <cstdint>

/**
 * Defines a target for teleportation
 */
class TeleportTarget {
	public:
		/** Construct an inactive target */
		TeleportTarget() = default;

		/** 
		 * Activate and set the target
		 * @param map_id map id
		 * @param x x position
		 * @param y y position
		 * @param d d direction, or -1 if retain direction.
		 */
		TeleportTarget(int map_id, int x, int y, int d);

		/** @return map id */
		int GetMapId() const;

		/** @return x position */
		int GetX() const;

		/** @return x position */
		int GetY() const;

		/** @return direction, or -1 if retain direction */
		int GetDirection() const;

		/** @return whether this is active */
		bool IsActive() const;
	private:
		int _map_id = 0;
		int _x = 0;
		int _y = 0;
		int16_t _d = -1;
		bool _active = false;
};


inline TeleportTarget::TeleportTarget(int map_id, int x, int y, int d)
	: _map_id(map_id)
	  , _x(x)
	  , _y(y)
	  , _d(d)
	  , _active(true)
{ }

inline int TeleportTarget::GetMapId() const {
	return _map_id;
}

inline int TeleportTarget::GetX() const {
	return _x;
}

inline int TeleportTarget::GetY() const {
	return _y;
}

inline int TeleportTarget::GetDirection() const {
	return _d;
}

inline bool TeleportTarget::IsActive() const {
	return _active;
}

#endif
