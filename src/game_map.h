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
#include <cstdint>
#include <vector>
#include <string>
#include "system.h"
#include "game_commonevent.h"
#include "game_event.h"
#include "game_vehicle.h"
#include "game_player.h"
#include <lcf/rpg/fwd.h>
#include <lcf/rpg/encounter.h>
#include <lcf/rpg/map.h>
#include <lcf/rpg/mapinfo.h>
#include <lcf/rpg/savemapinfo.h>
#include <lcf/rpg/savepanorama.h>
#include <lcf/rpg/savepartylocation.h>
#include <lcf/rpg/savevehiclelocation.h>
#include <lcf/rpg/savecommonevent.h>
#include "async_op.h"
#include <player.h>

class FileRequestAsync;
struct BattleArgs;

// These are in sixteenths of a pixel.
constexpr int SCREEN_TILE_SIZE = 256;

class MapUpdateAsyncContext {
	public:
		MapUpdateAsyncContext() = default;

		static MapUpdateAsyncContext FromCommonEvent(int ce, AsyncOp aop);
		static MapUpdateAsyncContext FromMapEvent(int ce, AsyncOp aop);
		static MapUpdateAsyncContext FromForegroundEvent(AsyncOp aop);
		static MapUpdateAsyncContext FromMessage(AsyncOp aop);

		AsyncOp GetAsyncOp() const;

		int GetParallelCommonEvent() const;
		int GetParallelMapEvent() const;

		bool IsForegroundEvent() const;
		bool IsParallelCommonEvent() const;
		bool IsParallelMapEvent() const;
		bool IsMessage() const;
		bool IsActive() const;
	private:
		AsyncOp async_op = {};
		int common_event = 0;
		int map_event = 0;
		bool foreground_event = false;
		bool message = false;
};

/**
 * Game_Map namespace
 */
namespace Game_Map {
	/**
	 * Initialize Game_Map.
	 */
	void Init();

	/**
	 * Initialized Common Events.
	 */
	void InitCommonEvents();

	/**
	 * Quits (frees) Game_Map.
	 */
	void Quit();

	/** Disposes Game_Map.  */
	void Dispose();

	/**
	 * Loads the map from disk
	 *
	 * @param map_id the id of the map to load
	 * @return the map, or nullptr if it couldn't be loaded
	 */
	std::unique_ptr<lcf::rpg::Map> loadMapFile(int map_id);

	/**
	 * Setups a new map.
	 *
	 * @pre Main_Data::game_player->GetMapId() reflects the new map.
	 */
	void Setup(std::unique_ptr<lcf::rpg::Map> map);

	/**
	 * Setups a map from a savegame.
	 *
	 * @param map - The map data
	 * @param save_map - The map state
	 * @param save_boat - The boat state
	 * @param save_ship - The ship state
	 * @param save_airship - The airship state
	 * @param save_fg_exec - The foreground interpreter state
	 * @param save_pan - The panorama state
	 * @param save_ce - The common event state
	 */
	void SetupFromSave(
			std::unique_ptr<lcf::rpg::Map> map,
			lcf::rpg::SaveMapInfo save_map,
			lcf::rpg::SaveVehicleLocation save_boat,
			lcf::rpg::SaveVehicleLocation save_ship,
			lcf::rpg::SaveVehicleLocation save_airship,
			lcf::rpg::SaveEventExecState save_fg_exec,
			lcf::rpg::SavePanorama save_pan,
			std::vector<lcf::rpg::SaveCommonEvent> save_ce);

	/**
	 * Copies event data into lcf::rpg::Save data.
	 *
	 * @param save - save data to populate.
	 */
	void PrepareSave(lcf::rpg::Save& save);

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
	 * Scrolls the map view in the given directions.
	 *
	 * @param dx amount to scroll along x-axis in 1/16th pixels
	 * @param dy amount to scroll along x-axis in 1/16th pixels
	 */
	void Scroll(int dx, int dy);

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
	 * @param from_x from tile x.
	 * @param from_y from tile y.
	 * @param to_x to new tile x.
	 * @param to_y to new tile y.
	 * @return whether is passable.
	 */
	bool MakeWay(const Game_Character& self,
			int from_x, int from_y,
			int to_x, int to_y);

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
	 * Return the tiles array for the given tiles layer.
	 *
	 * @param layer which layer to return
	 */
	std::vector<uint8_t> GetTilesLayer(int layer);

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
	 * Gets current map info.
	 *
	 * @return current map_info.
	 */
	const lcf::rpg::MapInfo& GetMapInfo();

	/**
	 * Gets map info of the specified map id.
	 *
	 * @param map_id map id
	 * @return map_info.
	 */
	const lcf::rpg::MapInfo& GetMapInfo(int map_id);

	/**
	 * Gets the map info of the parent map of the current map.
	 * The root of the tree has ID 0.
	 *
	 * @return parent map info
	 */
	const lcf::rpg::MapInfo& GetParentMapInfo();

	/**
	 * Gets map info of the parent map.
	 * The root of the tree has ID 0.
	 *
	 * @param map_info map info whose parent to retrieve
	 * @return parent map info
	 */
	const lcf::rpg::MapInfo& GetParentMapInfo(const lcf::rpg::MapInfo& map_info);

	/**
	 * Gets current map.
	 *
	 * @return current map.
	 */
	lcf::rpg::Map const& GetMap();

	/**
	 * Gets current map ID.
	 *
	 * @return current map ID.
	 */
	int GetMapId();

	/**
	 * Outputs the path in the map tree to reach the current map.
	 */
	void PrintPathToMap();

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

	/** @return original map battle encounter rate steps. */
	int GetOriginalEncounterSteps();

	/** @return battle encounter steps. */
	int GetEncounterSteps();

	/**
	 * Sets battle encounter steps.
	 *
	 * @param step encounter steps.
	 */
	void SetEncounterSteps(int step);

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
	 * @param args the arguments to pass to battle scene.
	 *
	 * @return true if battle starts, false if no monsters are at the current
	 * map position or encounter rate is 0
	 */
	bool PrepareEncounter(BattleArgs& args);

	/**
	 * Updates all battle data based on the current player position.
	 *
	 * @param args the arguments to pass to battle scene.
	 */
	void SetupBattle(BattleArgs& args);

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

	/** @return original map chipset ID */
	int GetOriginalChipset();

	/** @return current chipset ID */
	int GetChipset();

	/** @return chipset filename.  */
	StringView GetChipsetName();

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
	 *
	 * @param new_position_x new position x.
	 * @param reset_panorama whether to reset panorama
	 */
	void SetPositionX(int new_position_x, bool reset_panorama = true);

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
	 * @param reset_panorama whether to reset panorama
	 */
	void SetPositionY(int new_position_y, bool reset_panorama = true);

	/**
	 * @return need refresh flag.
	 */
	bool GetNeedRefresh();

	/**
	 * Gets the game interpreter.
	 *
	 * @return the game interpreter.
	 */
	Game_Interpreter_Map& GetInterpreter();

	/**
	 * Sets the need refresh flag.
	 *
	 * @param refresh need refresh flag.
	 */
	void SetNeedRefresh(bool refresh);

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

	/** @return highest event id present on the map, or 0 if no events */
	int GetHighestEventId();

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

	int RoundX(int x, int units = 1);
	int RoundY(int y, int units = 1);

	int RoundDx(int x, int units = 1);
	int RoundDy(int y, int units = 1);

	int XwithDirection(int x, int direction);
	int YwithDirection(int y, int direction);

	/**
	 * Gets the map name from MapInfo vector using map ID.
	 *
	 * @param id map ID.
	 * @return map name from MapInfo vector.
	 */
	StringView GetMapName(int id);

	/**
	 * Sets the chipset.
	 *
	 * @param id new chipset ID.
	 */
	void SetChipset(int id);

	bool ReloadChipset();

	void OnTranslationChanged();

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

	/**
	 * Gets whether there are any starting non-parallel event or common event.
	 * Used as a workaround for the Game Player.
	 *
	 * @return whether any starting non-parallel (common) event is starting
	 */
	bool IsAnyEventStarting();

	/** @return the number of times this map was saved in the editor */
	int GetMapSaveCount();

	/** @return true if any event on this map has an active move route */
	bool IsAnyMovePending();

	/** Cancel active move routes for all events on this map */
	void RemoveAllPendingMoves();

	void UpdateProcessedFlags(bool is_preupdate);
	bool UpdateCommonEvents(MapUpdateAsyncContext& actx);
	bool UpdateMapEvents(MapUpdateAsyncContext& actx);
	bool UpdateMessage(MapUpdateAsyncContext& actx);
	bool UpdateForegroundEvents(MapUpdateAsyncContext& actx);

	/**
	 * Construct a map name, either for EasyRPG or RPG Maker projects
	 *
	 * @param map_id The ID of the map to construct
	 * @param isEasyRpg Is the an easyrpg (emu) project, or an RPG Maker (lmu) one?
	 * @return The map name, as Map<map_id>.<map_extension>
	 */
	std::string ConstructMapName(int map_id, bool isEasyRpg);

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

		/** Adds x to the panorama x-position MOD parallax_width * TILE_SIZE * 2 */
		void AddPositionX(int x);

		/** Adds y to the panorama y-position MOD parallax_width * TILE_SIZE * 2 */
		void AddPositionY(int y);

		/** Sets the panorama x-position MOD parallax_width * TILE_SIZE * 2 */
		void SetPositionX(int x);

		/** Sets the panorama y-position MOD parallax_width * TILE_SIZE * 2 */
		void SetPositionY(int y);

		/** Reset the x position of the BG. */
		void ResetPositionX();

		/** Reset the x position of the BG. */
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

		/** @return Whether ox adjustment is required for fake resolution mode */
		bool FakeXPosition();

		/** @return Whether oy adjustment is required for fake resolution mode */
		bool FakeYPosition();
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

inline MapUpdateAsyncContext MapUpdateAsyncContext::FromMessage(AsyncOp aop) {
	MapUpdateAsyncContext actx;
	if (aop.IsActive()) {
		actx.async_op = aop;
		actx.message = true;
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

inline bool MapUpdateAsyncContext::IsMessage() const {
	return message;
}

inline bool MapUpdateAsyncContext::IsActive() const {
	return GetAsyncOp().IsActive();
}

#endif
