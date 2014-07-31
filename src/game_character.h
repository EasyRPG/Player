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
#include "color.h"
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
	virtual ~Game_Character();

	/**
	* Gets x position.
	*
	* @return x position.
	*/
	virtual int GetX() const = 0;

	/**
	* Sets x position.
	*
	* @param new_x new x position.
	*/
	virtual void SetX(int new_x) = 0;

	/**
	 * Gets y position.
	 *
	 * @return y position.
	 */
	virtual int GetY() const = 0;

	/**
	 * Sets y position.
	 *
	 * @param new_y new y position.
	 */
	virtual void SetY(int new_y) = 0;

	/**
	 * Gets the map id the character was inititialy on.
	 *
	 * @return map id.
	 */
	virtual int GetMapId() const = 0;

	/**
	 * Sets the map id the character was inititialy on.
	 *
	 * @param new_map_id New map id of character.
	 */
	virtual void SetMapId(int new_map_id) = 0;

	/**
	 * Gets character facing direction.
	 *
	 * @return current facing direction.
	 */
	virtual int GetDirection() const = 0;

	/**
	 * Sets character facing direction.
	 *
	 * @param new_direction New current facing direction.
	 */
	virtual void SetDirection(int new_direction) = 0;

	/**
	 * Gets facing direction before direction was locked.
	 *
	 * @return facing direction before lock.
	 */
	virtual int GetPrelockDirection() const = 0;

	/**
	 * Sets character facing used before locking.
	 *
	 * @param new_direction New prelock facing direction.
	 */
	virtual void SetPrelockDirection(int new_direction) = 0;

	/**
	 * Gets whether facing is locked.
	 *
	 * @return facing locked
	 */
	virtual bool IsFacingLocked() const = 0;

	/**
	 * Enables or disables locked facing direction.
	 *
	 * @param locked true: locked, false: unlocked.
	 */
	virtual void SetFacingLocked(bool locked) = 0;

	/**
	 * Gets the event layer (top, same, below).
	 *
	 * @return event layer
	 */
	virtual int GetLayer() const = 0;

	/**
	 * Sets the event layer (top, same, below).
	 *
	 * @param new_layer New event layer
	 */
	virtual void SetLayer(int new_layer) = 0;

	/**
	 * Gets character movement speed.
	 *
	 * @return character movement speed 
	 */
	virtual int GetMoveSpeed() const = 0;

	/**
	 * Sets character movement speed.
	 *
	 * @param speed new movement speed
	 */
	virtual void SetMoveSpeed(int speed) = 0;

	/**
	 * Gets character movement frequency.
	 *
	 * @return character movement frequency
	 */
	virtual int GetMoveFrequency() const = 0;

	/**
	 * Sets character movement frequency.
	 *
	 * @param frequency new character movement frequency
	 */
	virtual void SetMoveFrequency(int frequency) = 0;

	/**
	 * Returns the custom move route assigned via a MoveEvent.
	 *
	 * @return custom move route
	 */
	virtual const RPG::MoveRoute& GetMoveRoute() const = 0;

	/**
	 * Sets a new custom move route. Used to assign a new MoveEvent.
	 *
	 * @param move_route new custom move route
	 */
	virtual void SetMoveRoute(const RPG::MoveRoute& move_route) = 0;

	/**
	 * Returns current index of a "Movement Type Custom" move route.
	 * 
	 * @return current original move route index
	 */
	virtual int GetOriginalMoveRouteIndex() const = 0;

	/**
	 * Sets current index of a "Movement Type Custom" move route.
	 *
	 * @param new_index New move route index
	 */
	virtual void SetOriginalMoveRouteIndex(int new_index) = 0;

	/**
	 * Returns current index of the route assigned via a MoveEvent.
	 *
	 * @return current move route index
	 */
	virtual int GetMoveRouteIndex() const = 0;

	/**
	 * Sets current index of a MoveEvent move route.
	 *
	 * @param new_index New custom move route index
	 */
	virtual void SetMoveRouteIndex(int new_index) = 0;

	/**
	 * Gets whether move route is overwritten by event.
	 *
	 * @return move route overwritten
	 */
	virtual bool IsMoveRouteOverwritten() const = 0;

	/**
	 * Enables/Disables overwriting of move routes.
	 *
	 * @param force true: Use default move scheme, false: Use custom move route
	 */
	virtual void SetMoveRouteOverwritten(bool force) = 0;

	/**
	 * Checks if the forced move route has been repeating itself.
	 *
	 * @return whether forced move route has been repeating itself
	 */
	virtual bool IsMoveRouteRepeated() const = 0;

	/**
	 * Makes current forced move route repeated/non-repeated.
	 *
	 * @param repeat true: Repeated move route, false: Non-repeated move route
	 */
	virtual void SetMoveRouteRepeated(bool repeat) = 0;

	/**
	 * Gets sprite name. Usually the name of the graphic file.
	 *
	 * @return sprite name
	 */
	virtual const std::string& GetSpriteName() const = 0;

	/**
	 * Sets sprite name. Usually the name of the graphic file.
	 *
	 * @param sprite_name new sprite name
	 */
	virtual void SetSpriteName(const std::string& sprite_name) = 0;

	/**
	 * Gets sprite index of character.
	 *
	 * @return sprite index
	 */
	virtual int GetSpriteIndex() const = 0;

	/**
	 * Sets sprite index of character.
	 *
	 * @param index new sprite index
	 */
	virtual void SetSpriteIndex(int index) = 0;

	/**
	 * Gets flash effect color.
	 *
	 * @return flash color
	 */
	virtual Color GetFlashColor() const = 0;

	/**
	 * Sets flash effect color.
	 *
	 * @param flash_color new flash color
	 */
	virtual void SetFlashColor(const Color& flash_color) = 0;

	/**
	 * Returns intensity of flash effect.
	 *
	 * @return flash intensity
	 */
	virtual int GetFlashLevel() const = 0;

	/**
	 * Sets intensity of flash effect.
	 *
	 * @param flash_level new flash intensity
	 */
	virtual void SetFlashLevel(int flash_level) = 0;

	/**
	 * Returns how many flash effect time is left.
	 *
	 * @return time left
	 */
	virtual int GetFlashTimeLeft() const = 0;

	/**
	 * Set how long the flash effect will take.
	 *
	 * @param time_left flash duration
	 */
	virtual void SetFlashTimeLeft(int time_left) = 0;

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
	 * Gets if the character can walk in a tile
	 * with a specific direction.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @param d character direction.
	 * @return whether the character can walk through.
	 */
	virtual bool IsPassable(int x, int y, int d) const;

	/**
	 * Gets if the character can jump to a tile.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether the character can jump to.
	 */
	virtual bool IsLandable(int x, int y) const;

 	/**
	 * Gets if a message is halting this character's processes.
	 *
	 * @return whether the character is halted by a message.
	 */
	virtual bool IsMessageBlocking() const;

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
	 * Signals the owner of the current move route that the move ended.
	 * (or was overwritten by a new one)
	 */
	void EndMoveRoute();

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
	 * Moves the character diagonal (downleft), moves down if blocked.
	 */
	void MoveDownLeft();

	/**
	 * Moves the character diagonal (downright), moves down if blocked.
	 */
	void MoveDownRight();

	/**
	 * Moves the character diagonal (upleft), moves up if blocked.
	 */
	void MoveUpLeft();

	/**
	 * Moves the character diagonal (downright), moves up if blocked.
	 */
	void MoveUpRight();

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
	 * Signals a move begin.
	 */
	virtual void BeginMove();

	/**
	 * Character looks in a random direction
	 */
	void FaceRandomDirection();

	/**
	 * Character looks towards the hero.
	 */
	void TurnTowardHero();

	/**
	 * Character looks away from the the hero.
	 */
	void TurnAwayFromHero();

	/**
	 * Character waits for 20 frames more.
	 */
	void Wait();

	/**
	 * Jump action begins. Ends the movement when EndJump is missing.
	 *
	 * @param current_route Current move route
	 * @param current_index Index in the current route
	 *
	 * @return current_index if EndJump found, otherwise end of route.
	 */
	int BeginJump(const RPG::MoveRoute* current_route, int current_index);

	/**
	 * Jump action ends.
	 *
	 * @param current_route Current move route
	 * @param current_index Index in the current route
	 *
	 * @return current_index if jump was successful, else index of BeginJump.
	 */
	int EndJump(const RPG::MoveRoute* current_route, int current_index);

	/**
	 * Locks character facing direction.
	 */
	void Lock();

	/**
	 * Unlocks character facing direction.
	 */
	void Unlock();

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
	 * @param owner the interpreter which set the route.
	 */
	void CancelMoveRoute(Game_Interpreter* owner);

	/**
	 * Tells the character to not report back to the owner.
	 * (Usually because the owner got deleted).
	 *
	 * @param owner the owner of the move route;
	 *              if the owner is not the real owner
	 *              this function does nothing.
	 * @return true if the owner has been detached, false otherwise
	 */
	bool DetachMoveRouteOwner(Game_Interpreter* owner);

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
	 * Gets tile graphic ID.
	 *
	 * @return tile graphic ID.
	 */
	int GetTileId() const;

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
	 * Gets pattern.
	 *
	 * @return pattern.
	 */
	int GetPattern() const;

	/**
	 * Gets through flag.
	 *
	 * @return through flag.
	 */
	virtual bool GetThrough() const;

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

	int DistanceXfromPlayer() const;
	int DistanceYfromPlayer() const;

	virtual bool IsInPosition(int x, int y) const;

	virtual bool CheckEventTriggerTouch(int x, int y) = 0;

	/**
	 * Gets current opacity of character.
	 *
	 * @return opacity (0 = Invisible, 255 = opaque)
	 */
	virtual int GetOpacity() const;

	/**
	 * Sets opacity of the character.
	 *
	 * @param opacity New opacity (0 = Invisible, 255 = opaque)
	 */
	virtual void SetOpacity(int opacity);

	/**
	 * Gets if the character is visible.
	 *
	 * @return if visible, when true Opaque value is used
	 */
	virtual bool GetVisible() const;

	/**
	 * Makes character visible/not visible.
	 * This has a higher priority then the Opacity setting.
	 * Needed for the "SetHeroTransparency" command because this can't be
	 * altered via the "Increase Transparency" move command.
	 *
	 * @param visable true: visible, false: invisible
	 */
	virtual void SetVisible(bool visible);

	/**
	 * Gets whether a flash animation is pending for that character.
	 * A flash is pending when there is flash time left.
	 *
	 * @return Whether a flash is pending
	 */
	bool IsFlashPending() const;

	/**
	 * Tests if animation type is any fixed state or if the direction is locked.
	 *
	 * @return Whether direction is fixed
	 */
	bool IsDirectionFixed();

	/**
	 * Tests if animation type is any continuous state.
	 *
	 * @return Whether animation is continuous
	 */
	bool IsContinuous();

	/**
	 * Tests if animation is of the type spin.
	 *
	 * @return Whether animation is spin type
	 */
	bool IsSpinning();

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
	void UpdateJump();
	void UpdateSelfMovement();
	void UpdateStop();

	int tile_id;
	int real_x;
	int real_y;
	int pattern;
	int original_pattern;
	int last_pattern;
	bool through;
	int animation_id;
	int animation_type;
	
	RPG::MoveRoute original_move_route;
	Game_Interpreter* move_route_owner;
	int original_move_frequency;
	int move_type;
	bool move_failed;
	int move_count;
	int wait_count;

	bool jumping;
	int jump_peak;
	int jump_index;
	int jump_x;
	int jump_y;
	int jump_plus_x;
	int jump_plus_y;

	double anime_count;
	double stop_count;
	bool walk_animation;

	/** used by cycle left-right, up-down */
	bool cycle_stat;

	int opacity;
	bool visible;
};

#endif
