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
#include "async_op.h"

class FileRequestAsync;

// These are in sixteenths of a pixel.
constexpr int SCREEN_TILE_SIZE = 256;
constexpr int SCREEN_WIDTH = 20 * SCREEN_TILE_SIZE;
constexpr int SCREEN_HEIGHT = 15 * SCREEN_TILE_SIZE;

class MapUpdateAsyncContext {
	public:
		MapUpdateAsyncContext() = default;

		static MapUpdateAsyncContext FromCommonEvent(int ce, AsyncOp aop);
		static MapUpdateAsyncContext FromMapEvent(int ce, AsyncOp aop);
		static MapUpdateAsyncContext FromForegroundEvent(AsyncOp aop);

		AsyncOp GetAsyncOp() const;

		int GetParallelCommonEvent() const;
		int GetParallelMapEvent() const;

		bool IsForegroundEvent() const;
		bool IsParallelCommonEvent() const;
		bool IsParallelMapEvent() const;
		bool IsActive() const;
	private:
		int common_event = 0;
		int map_event = 0;
		AsyncOp async_op = {};
		bool foreground_event = 0;
};

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
	 *
	 * @param clear_screen clear the screen.
	 */
	void Dispose(bool clear_screen = true);

	/**
	 * Setups a map.
	 *
	 * @param map_id map ID.
	 * @param tt the type of teleport used to setup the map
	 */
	void Setup(int map_id, TeleportTarget::Type tt);

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

	/** Actions to perform after finishing a battle */
	void OnContinueFromBattle();

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

	/** @return how many sixteenths we scrolled right during this frame */
	int GetScrolledRight();

	/** @return how many sixteenths we scrolled down during this frame */
	int GetScrolledDown();

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
	 * @param actx asynchronous operations context. In out param.
	 *        If IsActive() when passed in, will resume to that point.
	 *        If IsActive() after return in, will suspend from that point.
	 * @param is_preupdate Update only common events and map events
	 */
	void Update(MapUpdateAsyncContext& actx, bool is_preupdate = false);

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
	 *
	 * @return true if an encounter should trigger.
	 */
	bool UpdateEncounterSteps();

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

	/**
	 * @param x x position on the map
	 * @param y y position on the map
	 * @param require_active If true, ignore events which are not active.
	 * @return the event with the highest id at (x,y)
	 */
	Game_Event* GetEventAt(int x, int y, bool require_active);

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

	/**
	 * Checks if the lower tile at (x,y) is passable by the player.
	 *
	 * Returns true if move is possible.
	 *
	 * @param bit which direction bits to check
	 * @param tile_index the tile index
	 * @return whether is passable.
	 */
	bool IsPassableLowerTile(int bit, int tile_index);

	enum PanDirection {
		PanUp,
		PanRight,
		PanDown,
		PanLeft
	};

	void LockPan();
	void UnlockPan();
	void StartPan(int direction, int distance, int speed);
	void ResetPan(int speed);
	void UpdatePan();

	/** @return how many frames it'll take to finish the current pan */
	int GetPanWait();

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
	bool IsPanLocked();
	int GetPanX();
	int GetPanY();
	int GetTargetPanX();
	int GetTargetPanY();

	void UpdateProcessedFlags(bool is_preupdate);
	bool UpdateCommonEvents(MapUpdateAsyncContext& actx);
	bool UpdateMapEvents(MapUpdateAsyncContext& actx);
	bool UpdateForegroundEvents(MapUpdateAsyncContext& actx);

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


inline AsyncOp MapUpdateAsyncContext::GetAsyncOp() const {
	return async_op;
}

inline MapUpdateAsyncContext MapUpdateAsyncContext::FromCommonEvent(int ce, AsyncOp aop) {
	MapUpdateAsyncContext actx;
	if (aop.IsActive()) {
		actx.async_op = aop;
		actx.common_event = ce;
	}
	return actx;
}

inline MapUpdateAsyncContext MapUpdateAsyncContext::FromMapEvent(int ev, AsyncOp aop) {
	MapUpdateAsyncContext actx;
	if (aop.IsActive()) {
		actx.async_op = aop;
		actx.map_event = ev;
	}
	return actx;
}

inline MapUpdateAsyncContext MapUpdateAsyncContext::FromForegroundEvent(AsyncOp aop) {
	MapUpdateAsyncContext actx;
	if (aop.IsActive()) {
		actx.async_op = aop;
		actx.foreground_event = true;
	}
	return actx;
}

inline int MapUpdateAsyncContext::GetParallelCommonEvent() const {
	return common_event;
}

inline int MapUpdateAsyncContext::GetParallelMapEvent() const {
	return map_event;
}

inline bool MapUpdateAsyncContext::IsForegroundEvent() const {
	return foreground_event;
}

inline bool MapUpdateAsyncContext::IsParallelCommonEvent() const {
	return common_event > 0;
}

inline bool MapUpdateAsyncContext::IsParallelMapEvent() const {
	return map_event > 0;
}


inline bool MapUpdateAsyncContext::IsActive() const {
	return GetAsyncOp().IsActive();
}

#endif
