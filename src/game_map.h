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

#ifndef EP_GAME_MAP_H
#define EP_GAME_MAP_H

// Headers
#include <vector>
#include <string>
#include "system.h"
#include "game_commonevent.h"
#include "game_event.h"
#include "game_vehicle.h"
#include "game_player.h"
#include "rpg_encounter.h"
#include "rpg_map.h"
#include "rpg_mapinfo.h"

class FileRequestAsync;

// These are in sixteenths of a pixel.
constexpr int SCREEN_TILE_SIZE = 256;
constexpr int SCREEN_WIDTH = 20 * SCREEN_TILE_SIZE;
constexpr int SCREEN_HEIGHT = 15 * SCREEN_TILE_SIZE;

/**
 * Game_Map namespace
 */
namespace Game_Map {
	enum RefreshMode {
		Refresh_None,
		Refresh_All,
		Refresh_Map
	};

	/**
	 * Initialize Game_Map.
	 */
	void Init();

	/**
	 * Quits (frees) Game_Map.
	 */
	void Quit();

	/**
	 * Disposes Game_Map.
	 */
	void Dispose();

	/**
	 * Setups a map.
	 *
	 * @param map_id map ID.
	 */
	void Setup(int map_id);

	/**
	 * Setups a map from a savegame.
	 */
	void SetupFromSave();

	/**
	 * Performs Setup on a map when teleport to self.
	 *
	 * @param map_id map ID.
	 */
	void SetupFromTeleportSelf();

	/**
	 * Shared code of the Setup methods.
	 *
	 * @param _id map ID.
	 */
	void SetupCommon(int _id, bool is_load_savegame);

	/**
	 * Copies event data into RPG::Save data.
	 */
	void PrepareSave();

	/**
	 * Runs map.
	 */
	void PlayBgm();

	/**
	 * Refreshes the map.
	 */
	void Refresh();

	/**
	 * Scrolls the map view right.
	 *
	 * @param distance scroll amount in sixteenths of a pixel
	 */
	void ScrollRight(int distance);

	/**
	 * Scrolls the map view down.
	 *
	 * @param distance scroll amount in sixteenths of a pixel
	 */
	void ScrollDown(int distance);

	/**
	 * Adds inc, a distance in sixteenths of a pixel, to screen_x, the
	 * x-position of a screen. The sum is clamped (for non-looping maps)
	 * and wrapped (for looping maps) to fit in the map range. If the
	 * sum is clamped, inc is updated to be actual amount that it
	 * changed by.
	 */
	void AddScreenX(int& screen_x, int& inc);

	/** Same as AddScreenX, but for the Y-direction. */
	void AddScreenY(int& screen_y, int& inc);

	/**
	 * Gets if a tile coordinate is valid.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether is valid.
	 */
	bool IsValid(int x, int y);

	/**
	 * Clears the way for a move by self to (x,y). Any events
	 * that block the way are updated early to give them a
	 * chance to move out of the way.
	 *
	 * Returns true if move is possible.
	 *
	 * @param self Character to move.
	 * @param x new tile x.
	 * @param y new tile y.
	 * @return whether is passable.
	 */
	bool MakeWay(const Game_Character& self, int x, int y);

	/**
	 * Gets if possible to land the airship at (x,y)
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether is posible to land airship
	 */
	bool CanLandAirship(int x, int y);

	/**
	 * Gets if possible to embark the boat or ship at (x,y)
	 *
	 * @param player the player
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether is posible to disembark the boat or ship
	 */
	bool CanEmbarkShip(Game_Player& player, int x, int y);

	/**
	 * Gets if possible to disembark the boat or ship to (x,y)
	 *
	 * @param player the player
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether is posible to disembark the boat or ship
	 */
	bool CanDisembarkShip(Game_Player& player, int x, int y);

	/**
	 * Gets the bush depth at a certain tile.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return bush depth.
	 */
	int GetBushDepth(int x, int y);

	/**
	 * Gets if a tile has counter flag.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether has the counter flag.
	 */
	bool IsCounter(int x, int y);

	/**
	 * Gets designated tile terrain tag.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return terrain tag ID.
	 */
	int GetTerrainTag(int x, int y);

	/**
	 * Gets designated position event.
	 *
	 * @param x : tile x
	 * @param y : tile y
	 * @return event id, 0 if no event found
	 */
	int CheckEvent(int x, int y);

	/**
	 * Updates the map state.
	 *
	 * @param is_preupdate Update only common events and map events
	 */
	void Update(bool is_preupdate = false);

	/**
	 * Gets current map_info.
	 *
	 * @return current map_info.
	 */
	RPG::MapInfo const& GetMapInfo();

	/**
	 * Gets current map.
	 *
	 * @return current map.
	 */
	RPG::Map const& GetMap();

	/**
	 * Gets current map ID.
	 *
	 * @return current map ID.
	 */
	int GetMapId();

	/**
	 * Gets current map width.
	 *
	 * @return current map width.
	 */
	int GetWidth();

	/**
	 * Gets current map height.
	 *
	 * @return current map height.
	 */
	int GetHeight();

	/**
	 * Gets battle encounters list.
	 *
	 * @return battle encounters list.
	 */
	std::vector<RPG::Encounter>& GetEncounterList();

	/**
	 * Gets battle encounter rate.
	 *
	 * @return battle encounter left steps.
	 */
	int GetEncounterRate();

	/**
	 * Sets battle encounter rate.
	 *
	 * @param step encounter steps.
	 */
	void SetEncounterRate(int step);

	/**
	 * Gets encounter steps.
	 *
	 * @return number of steps scaled by terrain encounter rate percentage.
	 */
	int GetEncounterSteps();

	/**
	 * Updates encounter steps according to terrain.
	 */
	void UpdateEncounterSteps();

	/**
	 * Sets encounter_steps to steps.
	 */
	void SetEncounterSteps(int steps);

	/**
	 * Gets possible encounters at a location.
	 * Respects areas and terrain settings.
	 *
	 * @param x x position
	 * @param y y position
	 * @return Possible encounters
	 */
	std::vector<int> GetEncountersAt(int x, int y);

	/**
	 * Updates all battle data based on the current player position and starts
	 * a random encounter.
	 *
	 * @return true if battle starts, false if no monsters are at the current
	 * map position or encounter rate is 0
	 */
	bool PrepareEncounter();

	/**
	 * Updates all battle data based on the current player position.
	 */
	void SetupBattle();

	/**
	 * Plays the given animation against a character.
	 *
	 * @param animation_id the animation ID
	 * @param target_id the ID of the targeted character
	 * @param global whether to "show on the entire map"
	 */
	void ShowBattleAnimation(int animation_id, int target_id, bool global);

	/**
	 * Whether or not a battle animation is currently playing.
	 */
	bool IsBattleAnimationWaiting();

	/**
	 * Gets lower layer map data.
	 *
	 * @return lower layer map data.
	 */
	std::vector<short>& GetMapDataDown();

	/**
	 * Gets upper layer map data.
	 *
	 * @return upper layer map data.
	 */
	std::vector<short>& GetMapDataUp();

	/**
	 * Gets chipset Id.
	 *
	 * @return chipset ID
	 */
	int GetChipset();

	/**
	 * Gets chipset filename.
	 *
	 * @return chipset filename.
	 */
	std::string& GetChipsetName();

	/**
	 * Sets the chipset name.
	 *
	 * @param chipset_name new chipset name.
	 */
	void SetChipsetName(std::string chipset_name);

	/**
	 * Gets battleback filename.
	 *
	 * @return battleback filename.
	 */
	std::string& GetBattlebackName();

	/**
	 * Sets the battleback name.
	 *
	 * @param battleback_name new battleback name.
	 */
	void SetBattlebackName(std::string battleback_name);

	/**
	 * Gets the offset of the screen from the left edge
	 * of the map, ignoring screen shaking.
	 */
	int GetPositionX();

	/**
	 * Gets the offset of the screen from the left edge
	 * of the map, taking shaking into account.
	 */
	int GetDisplayX();

	/**
	 * Sets the offset of the screen from the left edge
	 * of the map, as given by GetPositionX.
	 */
	void SetPositionX(int new_position_x);

	/**
	 * Gets display y.
	 */
	int GetPositionY();

	/**
	 * Gets display y.
	 */
	int GetDisplayY();

	/**
	 * Sets the offset of the screen from the top edge
	 * of the map.
	 *
	 * @param new_position_y new position y.
	 */
	void SetPositionY(int new_position_y);

	/**
	 * Gets need refresh flag.
	 *
	 * @return need refresh flag.
	 */
	RefreshMode GetNeedRefresh();

	/**
	 * Gets the game interpreter.
	 *
	 * @return the game interpreter.
	 */
	Game_Interpreter_Map& GetInterpreter();

	/**
	 * Destroy an interpreter after all events and common events have been updated.
	 *
	 * @param interpreter to destroy.
	 */
	void ReserveInterpreterDeletion(std::shared_ptr<Game_Interpreter> interpreter);

	/**
	 * Sets the need refresh flag.
	 *
	 * @param refresh_mode need refresh state.
	 */
	void SetNeedRefresh(RefreshMode refresh_mode);

	/**
	 * Gets lower passages list.
	 *
	 * @return lower passages list.
	 */
	std::vector<unsigned char>& GetPassagesDown();

	/**
	 * Gets upper passages list.
	 *
	 * @return upper passages list.
	 */
	std::vector<unsigned char>& GetPassagesUp();

	/**
	 * Gets chipset animation type.
	 *
	 * @return chipset animation type.
	 */
	int GetAnimationType();

	/**
	 * Gets chipset animation speed.
	 *
	 * @return chipset animation speed.
	 */
	int GetAnimationSpeed();

	/**
	 * Gets events list.
	 *
	 * @return events list.
	 */
	std::vector<Game_Event>& GetEvents();

	/**
	 * Gets pointer to event.
	 *
	 * @param event_id event ID
	 * @return pointer to event.
	 */
	Game_Event* GetEvent(int event_id);

	/**
	 * Gets common events list.
	 *
	 * @return common events list.
	 */
	std::vector<Game_CommonEvent>& GetCommonEvents();

	void GetEventsXY(std::vector<Game_Event*>& events, int x, int y);

	bool LoopHorizontal();
	bool LoopVertical();

	int RoundX(int x);
	int RoundY(int y);

	int XwithDirection(int x, int direction);
	int YwithDirection(int y, int direction);

	/**
	 * Gets the map index from MapInfo vector using map ID.
	 *
	 * @param id map ID.
	 * @return map index from MapInfo vector.
	 */
	int GetMapIndex(int id);

	/**
	 * Gets the map name from MapInfo vector using map ID.
	 *
	 * @param id map ID.
	 * @return map name from MapInfo vector.
	 */
	std::string GetMapName(int id);

	/**
	 * Gets the type (1 = normal, 2 = area) of the map.
	 *
	 * @param map_id map id
	 * @return type of the map
	 */
	int GetMapType(int map_id);

	/**
	 * Gets the ID of the parent map.
	 * The root of the tree has ID 0.
	 *
	 * @param map_id map id
	 * @return parent map id
	 */
	int GetParentId(int map_id);

	/**
	 * Sets the chipset.
	 *
	 * @param id new chipset ID.
	 */
	void SetChipset(int id);

	Game_Vehicle* GetVehicle(Game_Vehicle::Type which);
	int SubstituteDown(int old_id, int new_id);
	int SubstituteUp(int old_id, int new_id);

	/**
	 * Checks if its possible to step onto the tile at (x,y)
	 * The check includes tile graphic events checks.
	 *
	 * Returns true if move is possible.
	 *
	 * @param self Character to move. If not nullptr, checks the vehicle type and performs vehicle specific checks if is vehicle.
	 *        Also ignores self in the event tile graphic checks if self is not nullptr.
	 * @param bit which direction bits to check
	 * @param x target tile x.
	 * @param y target tile y.
	 * @return whether is passable.
	 */
	bool IsPassableTile(const Game_Character* self, int bit, int x, int y);

	enum PanDirection {
		PanUp,
		PanRight,
		PanDown,
		PanLeft
	};

	void LockPan();
	void UnlockPan();
	void StartPan(int direction, int distance, int speed, bool wait);
	void ResetPan(int speed, bool wait);
	void UpdatePan();

	/**
	 * Gets whether there are any starting non-parallel event or common event.
	 * Used as a workaround for the Game Player.
	 *
	 * @return whether any starting non-parallel (common) event is starting
	 */
	bool IsAnyEventStarting();

	bool IsAnyMovePending();
	void AddPendingMove(Game_Character* character);
	void RemovePendingMove(Game_Character* character);
	void RemoveAllPendingMoves();

	bool IsPanActive();
	bool IsPanWaiting();
	bool IsPanLocked();
	int GetPanX();
	int GetPanY();
	int GetTargetPanX();
	int GetTargetPanY();

	/**
	 * Gets if pending teleportations will be ignored.
	 *
	 * @return true: teleport ignored, false: teleport processed normally
	 */
	bool IsTeleportDelayed();

	/**
	 * Enables/Disables the processing of teleports.
	 * This is used by Show/EraseScreen in Parallel processes to prevent
	 * too early execution of teleports (Show/EraseScreen don't yield the
	 * interpreter in RPG_RT).
	 *
	 * @param delay enable/disable delay
	 */
	void SetTeleportDelayed(bool delay);

	FileRequestAsync* RequestMap(int map_id);

	namespace Parallax {
		struct Params {
			std::string name;
			bool scroll_horz;
			bool scroll_horz_auto;
			int scroll_horz_speed;
			bool scroll_vert;
			bool scroll_vert_auto;
			int scroll_vert_speed;
		};

		/**
		 * The name of the current parallax graphic (or the empty string
		 * if none).
		 */
		std::string GetName();

		/**
		 * Offset in pixels of the bitmap at the top-left of the screen.
		 * (If the screen is shaking, at the top-left of where the screen
		 * would be if it weren't.)
		 */
		int GetX();

		/** Same a GetX(), but in the y-direction. */
		int GetY();

		/** Call this when you find out the width and height of the BG. */
		void Initialize(int width, int height);

		/** Reset the x position of the BG. */
		void ResetPositionX();

		/** Reset the y position of the BG. */
		void ResetPositionY();

		/**
		 * To be called when the map scrolls right.
		 *
		 * @param distance Amount of scroll in 1/16th pixels.
		 */
		void ScrollRight(int distance);

		/**
		 * To be called when the map scrolls down.
		 *
		 * @param distance Amount of scroll in 1/16th pixels.
		 */
		void ScrollDown(int distance);

		/** Update autoscrolling BG (call every frame). */
		void Update();

		/** Change BG (eg. with a "Change Parallax BG" command). */
		void ChangeBG(const Params& params);

		/**
		 * Remove any changed BG. The BG goes back to what was set in
		 * the map properties.
		 */
		void ClearChangedBG();
	}
}

#endif
