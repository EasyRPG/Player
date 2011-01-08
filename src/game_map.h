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
#include "game_event.h"
#include "rpg_encounter.h"

typedef std::map<int, Game_Event*> tEventHash;
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
	
	/// @return current map id
	int GetMapId();

	/// @return current map width
	int GetWidth();

	/// @return current map height
	int GetHeight();

	/// @return battle encounters list
	std::vector<RPG::Encounter>& GetEncounterList();

	/// @return battle encounter left steps
	int GetEncounterStep();

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

	/// @return panorama filename
	std::string& GetPanoramaName();

	/// Set the panorama name
	/// @param panorama_name : new panorama name
	void SetPanoramaName(std::string panorama_name);

	/// @return panorama animation type
	int GetPanoramaType();

	/// Set the panorama type
	/// @param panorama_type : new panorama type
	void SetPanoramaType(int panorama_type);

	/// @return panorama animation speed
	int GetPanoramaSpeed();

	/// Set the panorama speed
	/// @param panorama_speed : new panorama speed
	void SetPanoramaSpeed(int panorama_speed);

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

}

#endif