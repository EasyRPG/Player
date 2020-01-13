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
		/** The type of teleport */
		enum Type {
			/** A teleport command from a parallel event */
			eParallelTeleport,
			/** A teleport command from a foreground event*/
			eForegroundTeleport,
			/** An escape or teleport skill */
			eSkillTeleport,
			/**
			 * An immediate asynchronous teleport from a SetVehicleLocation() command.
			 * This is an RPG_RT bug we emulate because it is abused by some games.
			 */
			eAsyncQuickTeleport,
		};

		/** Construct an inactive target */
		TeleportTarget() = default;

		/** 
		 * Activate and set the target
		 * @param map_id map id
		 * @param x x position
		 * @param y y position
		 * @param d d direction, or -1 if retain direction.
		 * @param tt teleport type
		 */
		TeleportTarget(int map_id, int x, int y, int d, Type tt);

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

		/** @return teleport type */
		Type GetType() const;
	private:
		int map_id = 0;
		int x = 0;
		int y = 0;
		int16_t d = -1;
		uint8_t tt = eParallelTeleport;
		bool active = false;
};


inline TeleportTarget::TeleportTarget(int map_id, int x, int y, int d, Type tt)
	: map_id(map_id)
	  , x(x)
	  , y(y)
	  , d(d)
	  , tt(tt)
	  , active(true)
{ }

inline int TeleportTarget::GetMapId() const {
	return map_id;
}

inline int TeleportTarget::GetX() const {
	return x;
}

inline int TeleportTarget::GetY() const {
	return y;
}

inline int TeleportTarget::GetDirection() const {
	return d;
}

inline bool TeleportTarget::IsActive() const {
	return active;
}

inline TeleportTarget::Type TeleportTarget::GetType() const {
	return Type(tt);
}

#endif
