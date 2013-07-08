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

#ifndef _GAME_MAP_H_
#define _GAME_MAP_H_

// Headers
#include <vector>
#include <string>
#include "system.h"
#include "game_commonevent.h"
#include "game_event.h"
#include "game_vehicle.h"
#include "rpg_encounter.h"

typedef std::map<int, EASYRPG_SHARED_PTR<Game_Event> > tEventHash;
typedef std::map<int, EASYRPG_SHARED_PTR<Game_CommonEvent> > tCommonEventHash;

/**
 * Game_Map namespace
 */
namespace Game_Map {
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
	 * Runs map.
	 */
	void Autoplay();

	/**
	 * Refreshes the map.
	 */
	void Refresh();

	/**
	 * Scrolls the map view down.
	 *
	 * @param distance number of tiles to scroll.
	 */
	void ScrollDown(int distance);

	/**
	 * Scrolls the map view left.
	 *
	 * @param distance number of tiles to scroll.
	 */
	void ScrollLeft(int distance);

	/**
	 * Scrolls the map view right.
	 *
	 * @param distance number of tiles to scroll.
	 */
	void ScrollRight(int distance);

	/**
	 * Scrolls the map view up.
	 *
	 * @param distance number of tiles to scroll.
	 */
	void ScrollUp(int distance);

	/**
	 * Gets if a tile coordinate is valid.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether is valid.
	 */
	bool IsValid(int x, int y);

	/**
	 * Gets if a tile coordinate is passable in a direction.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @param d direction (0, 2, 4, 6, 8, 10).
	 *		    0,10 = determine if all directions are impassable.
	 * @param self_event Current character for doing passability check
	 * @return whether is passable.
	 */
	bool IsPassable(int x, int y, int d, const Game_Character* self_event = NULL);

	/**
	 * Gets if a tile has bush flag.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return whether has the bush flag.
	 */
	bool IsBush(int x, int y);

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
	 * Gets if a tile can land airship.
	 *
	 * @param x tile x.
	 * @param y tile y.
	 * @return terrain tag ID.
	 */
	bool AirshipLandOk(int x, int y);

	/**
	 * Gets designated position event.
	 *
	 * @param x : tile x
	 * @param y : tile y
	 * @return event id, 0 if no event found
	 */
	int CheckEvent(int x, int y);

	/**
	 * Starts map scrolling.
	 *
	 * @param direction scroll direction.
	 * @param distance scroll distance in tiles.
	 * @param speed scroll speed.
	 */
	void StartScroll(int direction, int distance, int speed);

	/**
	 * Gets if the map is currently scrolling.
	 *
	 * @return whether the map view is scrolling.
	 */
	bool IsScrolling();

	/**
	 * Updates the map state.
	 */
	void Update();

	/**
	 * Updates the scroll state.
	 */
	void UpdateScroll();

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
	 * Resets encounter step counter based on the encounter rate using
	 * Gaussian distribution.
	 */
	void ResetEncounterSteps();

	/**
	 * Gets possible encounters at a location. Also scans areas.
	 *
	 * @param out Possible encounters
	 */
	void GetEncountersAt(int x, int y, std::vector<int>& out);

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
	 * Gets display x.
	 *
	 * @return display x.
	 */
	int GetDisplayX();

	/**
	 * Sets the display x.
	 *
	 * @param display_x new display x.
	 */
	void SetDisplayX(int display_x);

	/**
	 * Gets display y.
	 *
	 * @return display y.
	 */
	int GetDisplayY();

	/**
	 * Sets the display y.
	 *
	 * @param display_y new display y.
	 */
	void SetDisplayY(int display_y);

	/**
	 * Gets need refresh flag.
	 *
	 * @return need refresh flag.
	 */
	bool GetNeedRefresh();

	/**
	 * Gets the game interpreter.
	 *
	 * @return the game interpreter.
	 */
	Game_Interpreter& GetInterpreter();

	/**
	 * Sets the need refresh flag.
	 *
	 * @param need_refresh need refresh state.
	 */
	void SetNeedRefresh(bool need_refresh);

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
	 * Gets terrain tags list.
	 *
	 * @return terrain tags list.
	 */
	std::vector<short>& GetTerrainTags();

	/**
	 * Gets events list.
	 *
	 * @return events list.
	 */
	tEventHash& GetEvents();

	/**
	 * Gets common events list.
	 *
	 * @return common events list.
	 */
	tCommonEventHash& GetCommonEvents();

	void GetEventsXY(std::vector<Game_Event*>& events, int x, int y);

	bool LoopHorizontal();
	bool LoopVertical();

	int RoundX(int x);
	int RoundY(int y);

	int XwithDirection(int x, int direction);
	int YwithDirection(int y, int direction);


	void SetParallaxName(const std::string& name);
	void SetParallaxScroll(bool horz, bool vert,
						   bool horz_auto, bool vert_auto,
						   int horz_speed, int vert_speed);

	/**
	 * Gets the map index from MapInfo vector using map ID.
	 *
	 * @param id map ID.
	 * @return map index from MapInfo vector.
	 */
	int GetMapIndex(int id);

	/**
	 * Sets the chipset.
	 *
	 * @param id new chipset ID.
	 */
	void SetChipset(int id);

	Game_Vehicle* GetVehicle(Game_Vehicle::Type which);
	void SubstituteDown(int old_id, int new_id);
	void SubstituteUp(int old_id, int new_id);

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

	bool IsPanActive();
	bool IsPanWaiting();
	bool IsPanLocked();
	int GetPanX();
	int GetPanY();

	void UpdateParallax();
	int GetParallaxX();
	int GetParallaxY();
	const std::string& GetParallaxName();
}

#endif
