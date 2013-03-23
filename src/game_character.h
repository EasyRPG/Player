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

#ifndef _GAME_CHARACTER_H_
#define _GAME_CHARACTER_H_

// Headers
#include <string>
#include "rpg_moveroute.h"

class Game_Event;
class Game_Player;
class Game_Interpreter;

/**
 * Game_Character class.
 */
class Game_Character {
public:
	/**
	 * Constructor.
	 */
	Game_Character();

	/**
	 * Destructor.
	 */
	virtual ~Game_Character() {}

	/**
	 * Gets if character is moving.
	 *
	 * @return whether the character is moving.
	 */
	virtual bool IsMoving() const;

	/**
	 * Checks if the character is jumping.
	 *
	 * @return whether the character is jumping.
	 */
	virtual bool IsJumping() const;

	/**
	 * Checks if the character is stopping.
	 *
	 * @return whether the character is stopping.
	 */
	virtual bool IsStopping() const;

	/**
	 * Gets if character the character can walk in a tile
	 * with a specific direction.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @param d character direction.
	 * @return whether the character can walk through.
	 */
	virtual bool IsPassable(int x, int y, int d) const;

	/**
	 * Moves the character to a new tile.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 */
	virtual void MoveTo(int x, int y);

	/**
	 * Updates character state and actions.
	 */
	virtual void Update();

	/**
	 * Moves on a random route.
	 */
	void MoveTypeRandom();

	/**
	 * Moves left to right and switches direction if the
	 * move failed.
	 */
	void MoveTypeCycleLeftRight();

	/**
	 * Moves up and down and switches direction if the
	 * move failed.
	 */
	void MoveTypeCycleUpDown();

	/**
	 * Walks to the player.
	 */
	void MoveTypeTowardsPlayer();

	/**
	 * Walks to the player.
	 */
	void MoveTypeAwayFromPlayer();

	/**
	 * Walks around on a custom move route.
	 */
	void MoveTypeCustom();

	/**
	 * Moves the character down.
	 */
	void MoveDown();

	/**
	 * Moves the character left.
	 */
	void MoveLeft();

	/**
	 * Moves the character right.
	 */
	void MoveRight();

	/**
	 * Moves the character up.
	 */
	void MoveUp();

	/**
	 * Moves the character forward.
	 */
	void MoveForward();

	/**
	 * Does a random movement.
	 */
	void MoveRandom();

	/**
	 * Does a move to the player hero.
	 */
	void MoveTowardsPlayer();

	/**
	 * Does a move away from the player hero.
	 */
	void MoveAwayFromPlayer();

	/**
	 * Turns the character down.
	 */
	void TurnDown();

	/**
	 * Turns the character left.
	 */
	void TurnLeft();

	/**
	 * Turns the character right.
	 */
	void TurnRight();

	/**
	 * Turns the character up.
	 */
	void TurnUp();

	/**
	 * Turns the character 90 Degree to the left.
	 */
	void Turn90DegreeLeft();

	/**
	 * Turns the character 90 Degree to the right.
	 */
	void Turn90DegreeRight();

	/**
	 * Turns the character by 180 degree
	 */
	void Turn180Degree();

	/**
	 * Turns the character 90 Degree to the left or right
	 * by using a random number.
	 */
	void Turn90DegreeLeftOrRight();

	/**
	 * Locks character direction.
	 */
	void Lock();

	void Unlock();

	void SetDirection(int direction);

	/**
	 * Forces a new, temporary, move route.
	 *
	 * @param new_route new move route.
	 * @param frequency frequency.
	 * @param owner the interpreter which set the route.
	 */
	void ForceMoveRoute(RPG::MoveRoute* new_route, int frequency, Game_Interpreter* owner);

	/**
	 * Cancels a previous forced move route.
	 *
	 * @param route previous move route.
	 * @param owner the interpreter which set the route.
	 */
	void CancelMoveRoute(RPG::MoveRoute* route, Game_Interpreter* owner);

	/**
	 * Tells the character to not report back to the owner.
	 * (Usually because the owner got deleted).
	 *
	 * @param owner the owner of the move route;
	 *              if the owner is not the real owner
	 *              this function does nothing.
	 */
	void DetachMoveRouteOwner(Game_Interpreter* owner);

	/**
	 * Gets screen x coordinate in pixels.
	 *
	 * @return screen x coordinate in pixels.
	 */
	virtual int GetScreenX() const;

	/**
	 * Gets screen y coordinate in pixels.
	 *
	 * @return screen y coordinate in pixels.
	 */
	virtual int GetScreenY() const;

	/**
	 * Gets screen z coordinate in pixels.
	 *
	 * @return screen z coordinate in pixels.
	 */
	virtual int GetScreenZ() const;

	/**
	 * Gets screen z coordinate in pixels.
	 *
	 * @param height character height.
	 * @return screen z coordinate in pixels.
	 */
	virtual int GetScreenZ(int height) const;

	/**
	 * Gets x position.
	 *
	 * @return x position.
	 */
	int GetX() const;

	/**
	 * Gets y position.
	 *
	 * @return y position.
	 */
	int GetY() const;

	/**
	 * Gets tile graphic ID.
	 *
	 * @return tile graphic ID.
	 */
	int GetTileId() const;

	/**
	 * Gets character graphic filename.
	 *
	 * @return character graphic filename.
	 */
	std::string GetCharacterName() const;

	/**
	 * Gets character graphic index.
	 *
	 * @return character graphic index.
	 */
	int GetCharacterIndex() const;

	/**
	 * Gets real x.
	 *
	 * @return real x.
	 */
	int GetRealX() const;

	/**
	 * Gets real y.
	 *
	 * @return real y.
	 */
	int GetRealY() const;

	/**
	 * Gets facing direction.
	 *
	 * @return facing direction.
	 */
	int GetDirection() const;

	/**
	 * Gets pattern.
	 *
	 * @return pattern.
	 */
	int GetPattern() const;

	/**
	 * Gets move router forcing flag.
	 *
	 * @return move route forcing flag.
	 */
	bool GetMoveRouteForcing() const;

	/**
	 * Gets through flag.
	 *
	 * @return through flag.
	 */
	bool GetThrough() const;

	/**
	 * Gets animation ID.
	 *
	 * @return animation ID.
	 */
	int GetAnimationId() const;

	/**
	 * Sets animation ID.
	 *
	 * @param animation_id new animation ID.
	 */
	void SetAnimationId(int animation_id);

	void TurnTowardPlayer();

	int DistanceXfromPlayer() const;
	int DistanceYfromPlayer() const;

	virtual bool IsInPosition(int x, int y) const;
	int GetPriorityType() const;

	virtual bool CheckEventTriggerTouch(int x, int y) = 0;

	virtual bool IsTransparent() const;

	virtual void UpdateBushDepth();

	void SetGraphic(const std::string& name, int index);

	enum CharsID {
		CharPlayer		= 10001,
		CharBoat		= 10002,
		CharShip		= 10003,
		CharAirship		= 10004,
		CharThisEvent	= 10005
	};

	static Game_Character* GetCharacter(int character_id, int event_id);

protected:
	void UpdateMove();
	void UpdateSelfMovement();
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
	int original_direction;
	int original_pattern;
	int last_pattern;
	bool move_route_forcing;
	bool through;
	int animation_id;
	
	RPG::MoveRoute* move_route;
	RPG::MoveRoute* original_move_route;
	int move_route_index;
	Game_Interpreter* move_route_owner;
	int original_move_route_index;
	int original_move_frequency;
	int move_type;
	int move_speed;
	int move_frequency;
	int prelock_direction;
	bool move_failed;
	bool locked;
	int wait_count;

	double anime_count;
	int stop_count;
	int jump_count;
	bool step_anime;
	bool walk_anime;
	bool turn_enabled;
	bool direction_fix;

	/** used by cycle left-right, up-down */
	bool cycle_stat;

	int priority_type;
	bool transparent;
};

#endif
