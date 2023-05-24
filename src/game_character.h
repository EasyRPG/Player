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

#ifndef EP_GAME_CHARACTER_H
#define EP_GAME_CHARACTER_H

// Headers
#include <cstdint>
#include <string>
#include "color.h"
#include "flash.h"
#include <lcf/rpg/moveroute.h>
#include <lcf/rpg/eventpage.h>
#include <lcf/rpg/savemapeventbase.h>
#include "drawable.h"
#include "utils.h"

/**
 * Game_Character class.
 */
class Game_Character {
public:
	using AnimType = lcf::rpg::EventPage::AnimType;

	enum Type {
		Event,
		Player,
		Vehicle
	};

	static StringView TypeToStr(Type t);

	/**
	 * Destructor.
	 */
	virtual ~Game_Character();

	/** @return the type of character this is */
	Type GetType() const;

	/**
	 * Gets x position in tiles.
	 *
	 * @return x position.
	 */
	int GetX() const;

	/**
	 * Sets x position in tiles.
	 *
	 * @param new_x new x position.
	 */
	void SetX(int new_x);

	/**
	 * Gets y position in tiles.
	 *
	 * @return y position.
	 */
	int GetY() const;

	/**
	 * Sets y position in tiles.
	 *
	 * @param new_y new y position.
	 */
	void SetY(int new_y);

	/**
	 * Gets the map id the character was inititialy on.
	 *
	 * @return map id.
	 */
	int GetMapId() const;

	/**
	 * Sets the map id the character was inititialy on.
	 *
	 * @param new_map_id New map id of character.
	 */
	void SetMapId(int new_map_id);

	/**
	 * Gets character's movement direction.
	 *
	 * @return current front direction.
	 */
	int GetDirection() const;

	/**
	 * Sets character's movement direction.
	 *
	 * @param new_direction New current front direction.
	 */
	void SetDirection(int new_direction);

	/**
	 * Gets character's visible facing direction.
	 *
	 * @return direction of the sprite.
	 */
	int GetFacing() const;

	/**
	 * Sets character's visible facing direction.
	 *
	 * @param new_facing New facing direction.
	 */
	void SetFacing(int new_facing);

	/**
	 * Gets whether facing is locked.
	 *
	 * @return facing locked
	 */
	bool IsFacingLocked() const;

	/**
	 * Enables or disables locked facing direction.
	 *
	 * @param locked true: locked, false: unlocked.
	 */
	void SetFacingLocked(bool locked);

	/**
	 * Gets the event layer (top, same, below).
	 *
	 * @return event layer
	 */
	int GetLayer() const;

	/**
	 * Sets the event layer (top, same, below).
	 *
	 * @param new_layer New event layer
	 */
	void SetLayer(int new_layer);

	/**
	 * Gets whether other events can be in the same tile.
	 *
	 * @return whether event overlap is forbidden.
	 */
	bool IsOverlapForbidden() const;

	/**
	 * Gets character movement speed.
	 *
	 * @return character movement speed
	 */
	int GetMoveSpeed() const;

	/**
	 * Sets character movement speed.
	 *
	 * @param speed new movement speed
	 */
	void SetMoveSpeed(int speed);

	/**
	 * Gets character movement frequency.
	 *
	 * @return character movement frequency
	 */
	int GetMoveFrequency() const;

	/**
	 * Sets character movement frequency.
	 *
	 * @param frequency new character movement frequency
	 */
	void SetMoveFrequency(int frequency);

	/**
	 * Returns the custom move route assigned via a MoveEvent.
	 *
	 * @return custom move route
	 */
	const lcf::rpg::MoveRoute& GetMoveRoute() const;

	/**
	 * Sets a new custom move route. Used to assign a new MoveEvent.
	 *
	 * @param move_route new custom move route
	 */
	void SetMoveRoute(const lcf::rpg::MoveRoute& move_route);

	/**
	 * Returns current index of the route assigned via a MoveEvent.
	 *
	 * @return current move route index
	 */
	int GetMoveRouteIndex() const;

	/**
	 * Sets current index of a MoveEvent move route.
	 *
	 * @param new_index New custom move route index
	 */
	void SetMoveRouteIndex(int new_index);

	/**
	 * Gets whether move route is overwritten by event.
	 *
	 * @return move route overwritten
	 */
	bool IsMoveRouteOverwritten() const;

	/**
	 * Enables/Disables overwriting of move routes.
	 *
	 * @param force true: Use default move scheme, false: Use custom move route
	 */
	void SetMoveRouteOverwritten(bool force);

	/**
	 * Checks if the forced move route has finished (repeated at least once).
	 *
	 * @return whether forced move route has finished.
	 */
	bool IsMoveRouteFinished() const;

	/**
	 * Marks the forced move route as finished (repeated at least once) or not finished.
	 *
	 * @param finished true: forced move route finished, false: not finished
	 */
	void SetMoveRouteFinished(bool finished);

	/**
	 * Gets sprite name. Usually the name of the graphic file.
	 *
	 * @return sprite name
	 */
	const std::string& GetSpriteName() const;

	/** @return true if this has a tile sprite */
	bool HasTileSprite() const;

	/**
	 * Sets sprite name. Usually the name of the graphic file.
	 *
	 * @param sprite_name new sprite name
	 * @param index the index of the new sprite.
	 */
	void SetSpriteGraphic(std::string sprite_name, int index);

	/**
	 * Sets sprite name from a move route command. Usually the name of the graphic file.
	 * This can be overridden to change behavior by child classes.
	 *
	 * @param sprite_name new sprite name
	 * @param index the index of the new sprite.
	 */
	virtual void MoveRouteSetSpriteGraphic(std::string sprite_name, int index);

	/**
	 * Gets sprite index of character.
	 *
	 * @return sprite index
	 */
	int GetSpriteIndex() const;

	/**
	 * Gets animation frame of character.
	 *
	 * @return anim_frame
	 */
	int GetAnimFrame() const;

	/**
	 * Sets animation frame of character.
	 *
	 * @param frame new anim_frame
	 */
	void SetAnimFrame(int frame);

	/**
	 * @return true if animation is paused.
	 */
	bool IsAnimPaused() const;

	/**
	 * Sets whether animation is paused.
	 *
	 * @param value whether to pause the animation.
	 */
	void SetAnimPaused(bool value);

	/**
	 * Updates the sprite facing direction based on current direction
	 */
	void UpdateFacing();

	/**
	 * Begins a flash.
	 *
	 * @param r red color
	 * @param g blue color
	 * @param b green color
	 * @param power power of the flash
	 * @param frames Duration of the flash in frames
	 */
	void Flash(int r, int g, int b, int power, int frames);

	/**
	 * Gets flash effect color.
	 *
	 * @return flash color
	 */
	Color GetFlashColor() const;

	/**
	 * Returns intensity of flash effect.
	 *
	 * @return flash intensity
	 */
	double GetFlashLevel() const;

	/**
	 * Sets intensity of flash effect.
	 *
	 * @param flash_level new flash intensity
	 */
	void SetFlashLevel(double flash_level);

	/**
	 * Returns how many flash effect time is left.
	 *
	 * @return time left
	 */
	int GetFlashTimeLeft() const;

	/**
	 * Set how long the flash effect will take.
	 *
	 * @param time_left flash duration
	 */
	void SetFlashTimeLeft(int time_left);

	/**
	 * Gets the through flag (walk through everything)
	 *
	 * @return through flag
	 */
	bool GetThrough() const;

	/**
	 * Sets the through flag (walk through everything)
	 *
	 * @param through through flag
	 */
	void SetThrough(bool through);

	/** Resets the through flag to the move_route_through flag */
	void ResetThrough();

	/**
	 * @return stop_count
	 */
	int GetStopCount() const;

	/**
	 * Sets the stop_count
	 *
	 * @param sc the new stop count
	 */
	void SetStopCount(int sc);

	/**
	 * @return max_stop_count
	 */
	int GetMaxStopCount() const;

	/**
	 * @return stepping max_stop_count for the given frequency
	 * @param freq input movement frequency
	 */
	static constexpr int GetMaxStopCountForStep(int freq);

	/**
	 * @return turning max_stop_count for the given frequency
	 * @param freq input movement frequency
	 */
	static constexpr int GetMaxStopCountForTurn(int freq);

	/**
	 * @return waiting max_stop_count for the given frequency
	 * @param freq input movement frequency
	 */
	static constexpr int GetMaxStopCountForWait(int freq);

	/**
	 * @return the number of frames for animating steps while not moving
	 * @param speed the movement speed.
	 */
	static constexpr int GetStationaryAnimFrames(int speed);

	/**
	 * @return the number of frames for animating steps while moving or continuous.
	 * @param speed the movement speed.
	 */
	static constexpr int GetContinuousAnimFrames(int speed);

	/**
	 * @return the number of frames for animating steps while spinning.
	 * @param speed the movement speed.
	 */
	static constexpr int GetSpinAnimFrames(int speed);

	/**
	 * Sets the max_stop_count
	 *
	 * @param sc the new max stop count
	 */
	void SetMaxStopCount(int sc);

	/** @return true if waiting for stop count in movement to complete */
	bool IsStopCountActive() const;

	/**
	 * @return anim_count
	 */
	int GetAnimCount() const;

	/**
	 * Sets the stop_count
	 *
	 * @param ac the new anim count.
	 */
	void SetAnimCount(int ac);

	/** Resets the stepping animation */
	void ResetAnimation();

	/**
	 * Gets if character is moving.
	 *
	 * @return whether the character is moving.
	 */
	bool IsMoving() const;

	/**
	 * @return whether the character is jumping.
	 */
	bool IsJumping() const;

	/**
	 * Set whether the character is jumping.
	 *
	 * @param val flag indicating jumping status
	 */
	void SetJumping(bool val);

	/**
	 * @return X position where jump began.
	 */
	int GetBeginJumpX() const;

	/**
	 * Set X position where jump began.
	 *
	 * @param x x position where jump began
	 */
	void SetBeginJumpX(int x);

	/**
	 * @return Y position where jump began.
	 *
	 * @param y y position where jump began
	 */
	int GetBeginJumpY() const;

	/**
	 * Set Y position where jump began.
	 */
	void SetBeginJumpY(int y);

	/**
	 * @return whether the character is flying.
	 */
	bool IsFlying() const;

	/**
	 * Set whether the character is flying.
	 *
	 * @param val whether or not character is flying.
	 */
	void SetFlying(bool val);

	/**
	 * @return whether the RPG_RT processed flag is set.
	 */
	bool IsProcessed() const;

	/**
	 * Set the RPG_RT processed flag
	 */
	void SetProcessed(bool val);

	/**
	 * @return whether the event is paused.
	 */
	bool IsPaused() const;

	/**
	 * Set the paused flag
	 */
	void SetPaused(bool val);

	/**
	 * Activates or deactivates the event.
	 *
	 * @param active enables or disables the event.
	 */
	void SetActive(bool active);

	/**
	 * Gets if the event is active.
	 *
	 * @return if the event is active (or inactive via EraseEvent-EventCommand).
	 */
	bool IsActive() const;

	/**
	 * Checks if the character is stopping.
	 *
	 * @return whether the character is stopping.
	 */
	bool IsStopping() const;

	/**
	 * Moves the character to a new location.
	 *
	 * @param map_id map id
	 * @param x tile x.
	 * @param y tile y.
	 */
	virtual void MoveTo(int map_id, int x, int y);

	/**
	 * Move in the direction dir.
	 *
	 * @param dir the direction to move to.
	 *
	 * @return Whether move was successful or a move or jump is already in progress.
	 * @post If successful, IsStopping() == false.
	 */
	virtual bool Move(int dir);

	/**
	 * Jump to (x, y)
	 *
	 * @param x the x position to jump to.
	 * @param y the y position to jump to.
	 *
	 * @return Whether jump was successful or a move or jump is already in progress.
	 * @post If successful, IsStopping() == false.
	 */
	bool Jump(int x, int y);

	/**
	 * Check if this can move to the given tile.
	 *
	 * @param from_x Moving from x position
	 * @param from_y Moving from y position
	 * @param to_x Moving from x position
	 * @param to_y Moving from y position
	 *
	 * @return true if this can occupy (to_x, to_y) from (from_x, from_y)
	 */
	virtual bool MakeWay(int from_x, int from_y, int to_x, int to_y);

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

	/** @return the direction we would need to face the hero. */
	int GetDirectionToHero();

	/** @return the direction we would need to face away from hero. */
	int GetDirectionAwayHero();

	/**
	 * @param dir input direction
	 *
	 * @return the direction 90 degrees to the left of dir
	 */
	static constexpr int GetDirection90DegreeLeft(int dir);

	/**
	 * @param dir input direction
	 *
	 * @return the direction 90 degrees to the right of dir
	 */
	static constexpr int GetDirection90DegreeRight(int dir);

	/**
	 * @param dir input direction
	 *
	 * @return the direction 180 degrees to the of dir
	 */
	static constexpr int GetDirection180Degree(int dir);

	/**
	 * Character looks in a random direction
	 */
	void TurnRandom();

	/**
	 * Character looks towards the hero.
	 */
	void TurnTowardHero();

	/**
	 * Character looks away from the hero.
	 */
	void TurnAwayFromHero();

	/**
	 * Character waits for 20 frames more.
	 */
	void Wait();

	/**
	 * Forces a new, temporary, move route.
	 *
	 * @param new_route new move route.
	 * @param frequency frequency.
	 */
	void ForceMoveRoute(const lcf::rpg::MoveRoute& new_route, int frequency);

	/**
	 * Cancels a previous forced move route.
	 */
	void CancelMoveRoute();

	/** @return height of active jump in pixels */
	int GetJumpHeight() const;

	/**
	 * Gets sprite x coordinate transformed to screen coordinate in pixels.
	 *
	 * @param apply_shift When true the coordinate is shifted by the map width (for looping maps)
	 * @return screen x coordinate in pixels.
	 */
	virtual int GetScreenX(bool apply_shift = false) const;

	/**
	 * Gets sprite y coordinate transformed to screen coordinate in pixels.
	 *
	 * @param apply_shift When true the coordinate is shifted by the map height (for looping maps)
	 * @param apply_jump Apply jump height modifier if character is jumping
	 * @return screen y coordinate in pixels.
	 */
	virtual int GetScreenY(bool apply_shift = false, bool apply_jump = true) const;

	/**
	 * Gets screen z coordinate
	 *
	 * @param apply_shift Forwarded to GetScreenY
	 * @return screen z coordinate
	 */
	virtual Drawable::Z_t GetScreenZ(bool apply_shift = false) const;

	/**
	 * Gets tile graphic ID.
	 *
	 * @return tile graphic ID.
	 */
	int GetTileId() const;

	/**
	 * Gets sprite x position in pixels.
	 *
	 * @return real x.
	 */
	int GetSpriteX() const;

	/**
	 * Gets sprite y position in pixels.
	 *
	 * @return real y.
	 */
	int GetSpriteY() const;

	/**
	 * Gets remaining step
	 *
	 * @return remaining step
	 */
	int GetRemainingStep() const;

	/**
	 * Sets remaining step
	 *
	 * @param step new remaining step count
	 */
	void SetRemainingStep(int step);

	/**
	 * Gets animation type.
	 *
	 * @return animation type.
	 */
	AnimType GetAnimationType() const;

	/**
	 * Sets animation type.
	 *
	 * @param anim_type new animation type.
	 */
	void SetAnimationType(AnimType anim_type);

	int DistanceXfromPlayer() const;
	int DistanceYfromPlayer() const;

	/**
	 * Tests if the character is currently on the tile at x/y or moving
	 * towards it.
	 *
	 * @param x X tile position
	 * @param y Y tile position
	 * @return If on tile or moving towards
	 */
	virtual bool IsInPosition(int x, int y) const;

	/**
	 * Gets current opacity of character.
	 *
	 * @return opacity (0 = Invisible, 255 = opaque)
	 */
	int GetOpacity() const;

	/**
	 * @return RPG_RT transparency
	 */
	int GetTransparency() const;

	/**
	 * Sets transparency of the character.
	 *
	 * @param value New transparency (0 = Opaque, 7 = mostly transparent)
	 */
	void SetTransparency(int value);

	/**
	 * Gets if the character is visible.
	 *
	 * @return if visible, when true Opaque value is used
	 */
	virtual bool IsVisible() const;

	/**
	 * Makes character visible/not visible.
	 * This has a higher priority then the Opacity setting.
	 * Needed for the "SetHeroTransparency" command because this can't be
	 * altered via the "Increase Transparency" move command.
	 *
	 * @param hidden true: invisible, false: visible
	 */
	void SetSpriteHidden(bool hidden);

	/** @return true if sprite is hidden */
	bool IsSpriteHidden() const;

	/**
	 * Tests if animation type is any fixed state.
	 *
	 * @return Whether direction is fixed
	 */
	static constexpr bool IsDirectionFixedAnimationType(AnimType);

	/**
	 * Tests if the step animation is enabled.
	 * The animation is enabled when the animation type is not "fixed_graphic" and when
	 * the animation was not disabled through the move command "stop animation".
	 *
	 * @return Wheter animations are enabled.
	 */
	bool IsAnimated() const;

	/**
	 * Tests if animation type is any continuous state.
	 *
	 * @return Whether animation is continuous
	 */
	bool IsContinuous() const;

	/**
	 * Tests if animation is of the type spin.
	 *
	 * @return Whether animation is spin type
	 */
	bool IsSpinning() const;

	/**
	 * Gets the bush depth of the tile where this character is standing
	 *
	 * @return Bush depth at this character's position
	 */
	int GetBushDepth() const;

	enum CharsID {
		CharPlayer		= 10001,
		CharBoat		= 10002,
		CharShip		= 10003,
		CharAirship		= 10004,
		CharThisEvent	= 10005
	};

	enum Direction {
		Up = 0,
		Right,
		Down,
		Left,
		UpRight,
		DownRight,
		DownLeft,
		UpLeft
	};

	static bool IsDirectionDiagonal(int d);

	/** Reverses a direction, ex: ReverseDir(Up) == Down. */
	static int ReverseDir(int dir);

	static Game_Character* GetCharacter(int character_id, int event_id);

	static constexpr int GetDxFromDirection(int dir);
	static constexpr int GetDyFromDirection(int dir);

protected:
	explicit Game_Character(Type type, lcf::rpg::SaveMapEventBase* d);
	/** Check for and fix incorrect data after loading save game */
	void SanitizeData(StringView name);
	/** Check for and fix incorrect move route data after loading save game */
	void SanitizeMoveRoute(StringView name, const lcf::rpg::MoveRoute& mr, int32_t& idx, StringView chunk_name);
	void Update();
	virtual void UpdateAnimation();
	virtual void UpdateNextMovementAction() = 0;
	virtual void UpdateMovement(int amount);

	void SetMaxStopCountForStep();
	void SetMaxStopCountForTurn();
	void SetMaxStopCountForWait();
	void UpdateMoveRoute(int32_t& current_index, const lcf::rpg::MoveRoute& current_route, bool is_overwrite);
	void IncAnimCount();
	void IncAnimFrame();
	void UpdateFlash();
	bool BeginMoveRouteJump(int32_t& current_index, const lcf::rpg::MoveRoute& current_route);

	lcf::rpg::SaveMapEventBase* data();
	const lcf::rpg::SaveMapEventBase* data() const;

	int original_move_frequency = 2;
	// contains if any movement (<= step_forward) of a forced move route was successful

	Type _type = {};
	lcf::rpg::SaveMapEventBase* _data = nullptr;
};

template <typename T>
class Game_CharacterDataStorage : public Game_Character
{
	public:
		using Type = Game_Character::Type;
		Game_CharacterDataStorage(Type typ);

		Game_CharacterDataStorage(const Game_CharacterDataStorage&) = delete;
		Game_CharacterDataStorage& operator=(const Game_CharacterDataStorage&) = delete;

		Game_CharacterDataStorage(Game_CharacterDataStorage&&) noexcept;
		Game_CharacterDataStorage& operator=(Game_CharacterDataStorage&&) noexcept;

		~Game_CharacterDataStorage() = default;

		T* data();
		const T* data() const;
	private:
		T _data = {};
};

constexpr bool Game_Character::IsDirectionFixedAnimationType(AnimType at) {
	return
		at == lcf::rpg::EventPage::AnimType_fixed_continuous ||
		at == lcf::rpg::EventPage::AnimType_fixed_graphic ||
		at == lcf::rpg::EventPage::AnimType_fixed_non_continuous;
}

inline lcf::rpg::SaveMapEventBase* Game_Character::data() {
	return _data;
}

inline const lcf::rpg::SaveMapEventBase* Game_Character::data() const {
	return _data;
}

inline Game_Character::Type Game_Character::GetType() const {
	return _type;
}

inline int Game_Character::GetX() const {
	return data()->position_x;
}

inline void Game_Character::SetX(int new_x) {
	data()->position_x = new_x;
}

inline int Game_Character::GetY() const {
	return data()->position_y;
}

inline void Game_Character::SetY(int new_y) {
	data()->position_y = new_y;
}

inline int Game_Character::GetMapId() const {
	return data()->map_id;
}

inline void Game_Character::SetMapId(int new_map_id) {
	data()->map_id = new_map_id;
}

inline int Game_Character::GetDirection() const {
	return data()->direction;
}

inline void Game_Character::SetDirection(int new_direction) {
	data()->direction = new_direction;
}

inline int Game_Character::GetFacing() const {
	return data()->facing;
}

inline void Game_Character::SetFacing(int new_facing) {
	data()->facing = new_facing;
}

inline bool Game_Character::IsFacingLocked() const {
	return data()->lock_facing;
}

inline void Game_Character::SetFacingLocked(bool locked) {
	data()->lock_facing = locked || IsDirectionFixedAnimationType(GetAnimationType());
}

inline int Game_Character::GetLayer() const {
	return data()->layer;
}

inline void Game_Character::SetLayer(int new_layer) {
	data()->layer = new_layer;
}

inline bool Game_Character::IsOverlapForbidden() const {
	return data()->overlap_forbidden;
}

inline int Game_Character::GetMoveSpeed() const {
	return data()->move_speed;
}

inline void Game_Character::SetMoveSpeed(int speed) {
	data()->move_speed = speed;
}

inline int Game_Character::GetMoveFrequency() const {
	return data()->move_frequency;
}

inline void Game_Character::SetMoveFrequency(int frequency) {
	data()->move_frequency = frequency;
}

inline const lcf::rpg::MoveRoute& Game_Character::GetMoveRoute() const {
	return data()->move_route;
}

inline void Game_Character::SetMoveRoute(const lcf::rpg::MoveRoute& move_route) {
	data()->move_route = move_route;
}

inline int Game_Character::GetMoveRouteIndex() const {
	return data()->move_route_index;
}

inline void Game_Character::SetMoveRouteIndex(int new_index) {
	data()->move_route_index = new_index;
}

inline bool Game_Character::IsMoveRouteOverwritten() const {
	return data()->move_route_overwrite;
}

inline void Game_Character::SetMoveRouteOverwritten(bool force) {
	data()->move_route_overwrite = force;
}

inline bool Game_Character::IsMoveRouteFinished() const {
	return data()->move_route_finished;
}

inline void Game_Character::SetMoveRouteFinished(bool finished) {
	data()->move_route_finished = finished;
}

inline const std::string& Game_Character::GetSpriteName() const {
	return data()->sprite_name;
}

inline void Game_Character::SetSpriteGraphic(std::string sprite_name, int index) {
	data()->sprite_name = std::move(sprite_name);
	data()->sprite_id = index;
}

inline void Game_Character::MoveRouteSetSpriteGraphic(std::string sprite_name, int index) {
	SetSpriteGraphic(std::move(sprite_name), index);
}

inline int Game_Character::GetSpriteIndex() const {
	return data()->sprite_id;
}

inline int Game_Character::GetAnimFrame() const {
	return data()->anim_frame;
}

inline void Game_Character::SetAnimFrame(int frame) {
	data()->anim_frame = frame;
}

inline bool Game_Character::IsAnimPaused() const {
	return data()->anim_paused;
}

inline void Game_Character::SetAnimPaused(bool value) {
	data()->anim_paused = value;
}

inline Color Game_Character::GetFlashColor() const {
	return Flash::MakeColor(data()->flash_red, data()->flash_green, data()->flash_blue, data()->flash_current_level);
}

inline double Game_Character::GetFlashLevel() const {
	return data()->flash_current_level;
}

inline void Game_Character::SetFlashLevel(double flash_level) {
	data()->flash_current_level = flash_level;
}

inline int Game_Character::GetFlashTimeLeft() const {
	return data()->flash_time_left;
}

inline void Game_Character::SetFlashTimeLeft(int time_left) {
	data()->flash_time_left = time_left;
}

inline bool Game_Character::GetThrough() const {
	return data()->through;
}

inline void Game_Character::SetThrough(bool through) {
	data()->through = through;
}

inline void Game_Character::ResetThrough() {
	data()->through = data()->move_route_through;
}

inline Game_Character::AnimType Game_Character::GetAnimationType() const {
	return AnimType(data()->animation_type);
}

inline void Game_Character::SetAnimationType(Game_Character::AnimType anim_type) {
	data()->animation_type = int(anim_type);
	SetFacingLocked(IsDirectionFixedAnimationType(anim_type));
}

inline int Game_Character::GetStopCount() const {
	return data()->stop_count;
}

inline void Game_Character::SetStopCount(int sc) {
	data()->stop_count = sc;
}

inline int Game_Character::GetMaxStopCount() const {
	return data()->max_stop_count;
}

inline void Game_Character::SetMaxStopCount(int sc) {
	data()->max_stop_count = sc;
}

inline bool Game_Character::IsStopCountActive() const {
	return GetStopCount() < GetMaxStopCount();
}

inline int Game_Character::GetAnimCount() const {
	return data()->anim_count;
}

inline void Game_Character::SetAnimCount(int ac) {
	data()->anim_count = ac;
}

inline void Game_Character::IncAnimCount() {
	++data()->anim_count;
}

inline void Game_Character::IncAnimFrame() {
	data()->anim_frame = (data()->anim_frame + 1) % 4;
	SetAnimCount(0);
}

inline void Game_Character::ResetAnimation() {
	SetAnimCount(0);
	if (GetAnimationType() != lcf::rpg::EventPage::AnimType_fixed_graphic) {
		SetAnimFrame(lcf::rpg::EventPage::Frame_middle);
	}
}

inline int Game_Character::GetRemainingStep() const {
	return data()->remaining_step;
}

inline void Game_Character::SetRemainingStep(int step) {
	data()->remaining_step = step;
}

inline bool Game_Character::IsJumping() const {
	return data()->jumping;
}

inline void Game_Character::SetJumping(bool val) {
	data()->jumping = val;
}

inline bool Game_Character::IsMoving() const {
	return !IsJumping() && GetRemainingStep() > 0;
}

inline bool Game_Character::IsStopping() const {
	return !(IsMoving() || IsJumping());
}

inline int Game_Character::GetBeginJumpX() const {
	return data()->begin_jump_x;
}

inline void Game_Character::SetBeginJumpX(int x) {
	data()->begin_jump_x = x;
}

inline int Game_Character::GetBeginJumpY() const {
	return data()->begin_jump_y;
}

inline void Game_Character::SetBeginJumpY(int y) {
	data()->begin_jump_y = y;
}

inline bool Game_Character::IsFlying() const {
	return data()->flying;
}

inline void Game_Character::SetFlying(bool val) {
	data()->flying = val;
}

inline int Game_Character::GetTransparency() const {
	return data()->transparency;
}

inline void Game_Character::SetTransparency(int value) {
	data()->transparency = Utils::Clamp(value, 0, 7);
}

inline bool Game_Character::IsProcessed() const {
	return data()->processed;
}

inline void Game_Character::SetProcessed(bool val) {
	data()->processed = val;
}

inline bool Game_Character::IsPaused() const {
	return data()->pause;
}

inline void Game_Character::SetPaused(bool val) {
	data()->pause = val;
}

inline bool Game_Character::IsActive() const {
	return data()->active;
}

inline void Game_Character::SetActive(bool active) {
	data()->active = active;
}

inline bool Game_Character::HasTileSprite() const {
	return GetSpriteName().empty();
}

inline int Game_Character::GetTileId() const {
	return HasTileSprite() ? GetSpriteIndex() : 0;
}

inline void Game_Character::SetSpriteHidden(bool hidden) {
	data()->sprite_hidden = hidden;
}

inline bool Game_Character::IsSpriteHidden() const {
	return data()->sprite_hidden;
}

constexpr int Game_Character::GetDirection90DegreeLeft(int dir) {
	return (dir + 3) % 4;
}

constexpr int Game_Character::GetDirection90DegreeRight(int dir) {
	return (dir + 1) % 4;
}

constexpr int Game_Character::GetDirection180Degree(int dir) {
	return (dir + 2) % 4;
}

constexpr int Game_Character::GetMaxStopCountForStep(int freq) {
	return freq >= 8 ? 0 : 1 << (9 - freq);
}

constexpr int Game_Character::GetMaxStopCountForTurn(int freq) {
	return freq >= 8 ? 0 : 1 << (8 - freq);
}

constexpr int Game_Character::GetMaxStopCountForWait(int freq) {
	return 20 + GetMaxStopCountForTurn(freq);
}

constexpr int Game_Character::GetDxFromDirection(int dir) {
	return (dir == Game_Character::Right || dir == Game_Character::UpRight || dir == Game_Character::DownRight)
		- (dir == Game_Character::Left || dir == Game_Character::DownLeft || dir == Game_Character::UpLeft);
}

constexpr int Game_Character::GetDyFromDirection(int dir) {
	return (dir == Game_Character::Down || dir == Game_Character::DownRight || dir == Game_Character::DownLeft)
		- (dir == Game_Character::Up || dir == Game_Character::UpRight || dir == Game_Character::UpLeft);
}

constexpr int Game_Character::GetStationaryAnimFrames(int speed) {
	constexpr int limits[] = { 12, 10, 8, 6, 5, 4 };
	return limits[speed - 1];
}

constexpr int Game_Character::GetContinuousAnimFrames(int speed) {
	constexpr int limits[] = { 16, 12, 10, 8, 7, 6 };
	return limits[speed - 1];
}

constexpr int Game_Character::GetSpinAnimFrames(int speed) {
	constexpr int limits[] = { 24, 16, 12, 8, 6, 4 };
	return limits[speed - 1];
}

inline bool Game_Character::IsVisible() const {
	return IsActive() && !IsSpriteHidden() && GetOpacity() > 0;
}

template <typename T>
inline Game_CharacterDataStorage<T>::Game_CharacterDataStorage(Type typ)
	: Game_Character(typ, nullptr)
{
	Game_Character::_data = &this->_data;
}

template <typename T>
inline Game_CharacterDataStorage<T>::Game_CharacterDataStorage(Game_CharacterDataStorage&& o) noexcept
: Game_Character(std::move(o)), _data(std::move(o._data))
{
	Game_Character::_data = &this->_data;
}

template <typename T>
inline Game_CharacterDataStorage<T>& Game_CharacterDataStorage<T>::operator=(Game_CharacterDataStorage&& o) noexcept
{
	static_cast<Game_Character*>(this) = std::move(o);
	if (this != &o) {
		_data = std::move(o._data);
		Game_Character::_data = &this->_data;
	}
	return *this;
}

template <typename T>
inline T* Game_CharacterDataStorage<T>::data() {
	return static_cast<T*>(Game_Character::data());
}

template <typename T>
inline const T* Game_CharacterDataStorage<T>::data() const {
	return static_cast<const T*>(Game_Character::data());
}

inline bool Game_Character::IsDirectionDiagonal(int d) {
	return d >= UpRight;
}

inline StringView Game_Character::TypeToStr(Game_Character::Type type) {
	switch (type) {
		case Player: return "Player";
		case Vehicle: return "Vehicle";
		case Event: return "Event";
	}
	return "UnknownCharacter";
}

#endif
