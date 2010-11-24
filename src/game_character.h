/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _GAME_CHARACTER_H_
#define _GAME_CHARACTER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>

////////////////////////////////////////////////////////////
/// Game_Character class.
////////////////////////////////////////////////////////////
class Game_Character {
public:
	////////////////////////////////////////////////////////
	/// Constructor.
	////////////////////////////////////////////////////////
	Game_Character();

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~Game_Character() {}

	////////////////////////////////////////////////////////
	/// Get if character is moving.
	/// @return whether the character is moving
	////////////////////////////////////////////////////////
	virtual bool IsMoving() const;

	////////////////////////////////////////////////////////
	/// Get if character the character can walk in a tile
	/// with a specific direction.
	/// @param x : tile x
	/// @param y : tile y
	/// @param d : character direction
	/// @return whether the character can walk through
	////////////////////////////////////////////////////////
	virtual bool IsPassable(int x, int y, int d) const;

	////////////////////////////////////////////////////////
	/// Moves the character to a new tile.
	/// @param x : tile x
	/// @param y : tile y
	////////////////////////////////////////////////////////
	virtual void MoveTo(int x, int y);

	////////////////////////////////////////////////////////
	/// Updates character state and actions.
	////////////////////////////////////////////////////////
	virtual void Update();

	////////////////////////////////////////////////////////
	/// Move the character down.
	////////////////////////////////////////////////////////
	void MoveDown();

	////////////////////////////////////////////////////////
	/// Move the character left.
	////////////////////////////////////////////////////////
	void MoveLeft();

	////////////////////////////////////////////////////////
	/// Move the character right.
	////////////////////////////////////////////////////////
	void MoveRight();

	////////////////////////////////////////////////////////
	/// Move the character up.
	////////////////////////////////////////////////////////
	void MoveUp();

	////////////////////////////////////////////////////////
	/// Turn the character down.
	////////////////////////////////////////////////////////
	void TurnDown();

	////////////////////////////////////////////////////////
	/// Turn the character left.
	////////////////////////////////////////////////////////
	void TurnLeft();

	////////////////////////////////////////////////////////
	/// Turn the character right.
	////////////////////////////////////////////////////////
	void TurnRight();

	////////////////////////////////////////////////////////
	/// Turn the character up.
	////////////////////////////////////////////////////////
	void TurnUp();

	////////////////////////////////////////////////////////
	/// Locks character direction.
	////////////////////////////////////////////////////////
	void Lock();

	/// @return screen x coordinate in pixels
	virtual int GetScreenX() const;

	/// @return screen y coordinate in pixels
	virtual int GetScreenY() const;

	/// @return screen z coordinate in pixels
	virtual int GetScreenZ() const;

	/// @param height : character height
	/// @return screen z coordinate in pixels
	virtual int GetScreenZ(int height) const;

	/// @return x position
	int GetX() const;

	/// @return y position
	int GetY() const;

	/// @return tile graphic id
	int GetTileId() const;

	/// @return character graphic filename
	std::string GetCharacterName() const;

	/// @return character graphic index
	int GetCharacterIndex() const;

	/// @return real x
	int GetRealX() const;

	/// @return real y
	int GetRealY() const;

	/// @return facing direction
	int GetDirection() const;

	/// @return pattern
	int GetPattern() const;

	/// @return move route forcing flag
	bool GetMoveRouteForcing() const;

	/// @return through flag
	bool GetThrough() const;

	/// @return animation id
	int GetAnimationId() const;

	/// @param animation_id : new animation id
	void SetAnimationId(int animation_id);

protected:
	void UpdateMove();
	void UpdateStop();

	int x;
	int y;
	int tile_id;
	std::string character_name;
	int character_index;
	int real_x;
	int real_y;
	int direction;
	int pattern;
	bool move_route_forcing;
	bool through;
	int animation_id;
	
	int move_speed;
	int move_frequency;
	int prelock_direction;
	bool locked;

	double anime_count;
	int stop_count;
	int original_pattern;
	int last_pattern;
	bool step_anime;
	bool walk_anime;
	bool turn_enabled;
	bool direction_fix;
};

#endif
