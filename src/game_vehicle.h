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

#ifndef _GAME_VEHICLE_H_
#define _GAME_VEHICLE_H_

// Headers
#include <string>
#include "rpg_music.h"
#include "game_character.h"

/**
 * Game_Vehicle class.
 */
class Game_Vehicle : public Game_Character {
public:
	static const int MAX_ALTITUDE = 32;	// the airship flies at

	enum Type {
		Boat,
		Ship,
		Airship
	};

	Game_Vehicle(Type _type);
	void LoadSystemSettings();
	void Refresh();
	void SetPosition(int _map_id, int _x, int _y);
	virtual bool IsInPosition(int x, int y) const;
	virtual bool IsTransparent() const;
	void GetOn();
	void GetOff();
	void SyncWithPlayer();
	int GetSpeed() const;
	virtual int GetScreenY() const;
	bool IsMovable();
	virtual void Update();
	virtual bool CheckEventTriggerTouch(int x, int y);

protected:
	int map_id;
	Type type;
	int altitude;
	bool driving;
	RPG::Music bgm;
};

#endif
