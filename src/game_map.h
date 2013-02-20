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

#ifndef _GAME_MAP_H_
#define _GAME_MAP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include <string>
#include "system.h"
#include "game_commonevent.h"
#include "game_event.h"
#include "game_vehicle.h"
#include "rpg_encounter.h"

typedef std::map<int, EASYRPG_SHARED_PTR<Game_Event> > tEventHash;
typedef std::map<int, EASYRPG_SHARED_PTR<Game_CommonEvent> > tCommonEventHash;

////////////////////////////////////////////////////////////
/// Game_Map namespace
////////////////////////////////////////////////////////////
namespace Game_Map {
	////////////////////////////////////////////////////////
	/// Initialize Game_Map.
	////////////////////////////////////////////////////////
	void Init();

	////////////////////////////////////////////////////////
	/// Quits (frees) Game_Map.
	////////////////////////////////////////////////////////
	void Quit();

	////////////////////////////////////////////////////////
	/// Dispose Game_Map.
	////////////////////////////////////////////////////////
	void Dispose();

	////////////////////////////////////////////////////////
	/// Setups a map.
	/// @param map_id : map id.
	////////////////////////////////////////////////////////
	void Setup(int map_id);

	////////////////////////////////////////////////////////
	/// Runs map.
	////////////////////////////////////////////////////////
	void Autoplay();

	////////////////////////////////////////////////////////
	/// Refersh the map.
	////////////////////////////////////////////////////////
	void Refresh();

	////////////////////////////////////////////////////////
	/// Scrolls the map view down.
	/// @param distance : number of tiles to scroll
	////////////////////////////////////////////////////////
	void ScrollDown(int distance);

	////////////////////////////////////////////////////////
	/// Scrolls the map view left.
	/// @param distance : number of tiles to scroll
	////////////////////////////////////////////////////////
	void ScrollLeft(int distance);

	////////////////////////////////////////////////////////
	/// Scrolls the map view right.
	/// @param distance : number of tiles to scroll
	////////////////////////////////////////////////////////
	void ScrollRight(int distance);

	////////////////////////////////////////////////////////
	/// Scrolls the map view up.
	/// @param distance : number of tiles to scroll
	////////////////////////////////////////////////////////
	void ScrollUp(int distance);

	////////////////////////////////////////////////////////
	/// Get if a tile coordinate is valid.
	/// @param x : tile x
	/// @param y : tile y
	/// @return whether is valid
	////////////////////////////////////////////////////////
	bool IsValid(int x, int y);

	////////////////////////////////////////////////////////
	/// Get if a tile coordinate is passable in a direction.
	/// @param x : tile x
	/// @param y : tile y
	/// @param d : direction (0,2,4,6,8,10)
	///		0,10 = determine if all directions are impassable
	/// @param self_event : ignores self
	/// @return whether is passable
	////////////////////////////////////////////////////////
	bool IsPassable(int x, int y, int d, const Game_Character* self_event = NULL);

	////////////////////////////////////////////////////////
	/// Get if a tile has bush flag.
	/// @param x : tile x
	/// @param y : tile y
	/// @return whether has the bush flag
	////////////////////////////////////////////////////////
	bool IsBush(int x, int y);

	////////////////////////////////////////////////////////
	/// Get if a tile has counter flag.
	/// @param x : tile x
	/// @param y : tile y
	/// @return whether has the counter flag
	////////////////////////////////////////////////////////
	bool IsCounter(int x, int y);

	////////////////////////////////////////////////////////
	/// Get designated tile terrain tag.
	/// @param x : tile x
	/// @param y : tile y
	/// @return terrain tag id
	////////////////////////////////////////////////////////
	int GetTerrainTag(int x, int y);

	////////////////////////////////////////////////////////
	/// Get if a tile can land airship.
	/// @param x : tile x
	/// @param y : tile y
	/// @return terrain tag id
	////////////////////////////////////////////////////////
	bool AirshipLandOk(int x, int y);

	////////////////////////////////////////////////////////
	/// Get designated position event.
	/// @param x : tile x
	/// @param y : tile y
	/// @return event id, 0 if no event found
	////////////////////////////////////////////////////////
	int CheckEvent(int x, int y);

	////////////////////////////////////////////////////////
	/// Starts map scrolling.
	/// @param direction : scroll direction
	/// @param distance : scroll distance in tiles
	/// @param speed : scroll speed
	////////////////////////////////////////////////////////
	void StartScroll(int direction, int distance, int speed);

	////////////////////////////////////////////////////////
	/// Get if the map is currently scrolling.
	/// @return whether the map view is scrolling
	////////////////////////////////////////////////////////
	bool IsScrolling();

	////////////////////////////////////////////////////////
	/// Updates the map state.
	////////////////////////////////////////////////////////
	void Update();

	////////////////////////////////////////////////////////
	/// Updates the scroll state.
	////////////////////////////////////////////////////////
	void UpdateScroll();

	/// @return current map
	RPG::Map const& GetMap();

	/// @return current map id
	int GetMapId();

	/// @return current map width
	int GetWidth();

	/// @return current map height
	int GetHeight();

	/// @return battle encounters list
	std::vector<RPG::Encounter>& GetEncounterList();

	/// @return battle encounter left steps
	int GetEncounterRate();

	/// @param battle encounter steps
	void SetEncounterRate(int step);

	/// @return number of steps scaled by terrain encounter rate percentage
	int GetEncounterSteps();

	/// update encounter steps according to terrain
	void UpdateEncounterSteps();

	/// reset encounter step counter
	void ResetEncounterSteps();

	/// @return lower layer map data
	std::vector<short>& GetMapDataDown();

	/// @return upper layer map data
	std::vector<short>& GetMapDataUp();

	/// @return chipset filename
	std::string& GetChipsetName();

	/// Set the chipset name
	/// @param chipset_name : new chipset name
	void SetChipsetName(std::string chipset_name);

	/// @return battleback filename
	std::string& GetBattlebackName();

	/// Set the battleback name
	/// @param battleback_name : new battleback name
	void SetBattlebackName(std::string battleback_name);

	/// @return display x
	int GetDisplayX();

	/// Set the display x
	/// @param display_x : new display x
	void SetDisplayX(int display_x);

	/// @return display y
	int GetDisplayY();

	/// Set the display y
	/// @param display_y : new display y
	void SetDisplayY(int display_y);

	/// @return need refresh flag
	bool GetNeedRefresh();

	Game_Interpreter& GetInterpreter();

	/// Set the need refresh flag
	/// @param need_refresh : need refresh state
	void SetNeedRefresh(bool need_refresh);

	/// @return lower passages list
	std::vector<unsigned char>& GetPassagesDown();

	/// @return upper passages list
	std::vector<unsigned char>& GetPassagesUp();

	/// @return terrain tags list
	std::vector<short>& GetTerrainTags();

	/// @return events list
	tEventHash& GetEvents();

	/// @return common events list
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

	////////////////////////////////////////////////////////
	/// Get the map index from MapInfo vector using map id.
	/// @param id : map id.
	/// @return map index from MapInfo vector.
	////////////////////////////////////////////////////////
	int GetMapIndex(int id);

	/// Set the chipset
	/// @param id : new chipset ID
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
