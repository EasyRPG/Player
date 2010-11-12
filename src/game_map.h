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
#include "tone.h"
#include "rpg_map.h"

class Game_Event;

////////////////////////////////////////////////////////////
/// Game_Map class
////////////////////////////////////////////////////////////
class Game_Map {
public:
	Game_Map();
	~Game_Map();

	void Setup(int _id);
	int GetMapId() const;
	int GetWidth() const;
	int GetHeight() const;
	std::vector<int> GetEncounterList();
	int GetEncounterStep();
	std::vector<short> GetMapDataDown();
	std::vector<short> GetMapDataUp();
	void Autoplay();
	void Refresh();
	void ScrollDown(int distance);
	void ScrollLeft(int distance);
	void ScrollRight(int distance);
	void ScrollUp(int distance);
	bool IsValid(int x, int y);
	//bool IsPassable(int x, int y, int d, Game_Event* self_event = NULL);
	bool IsBush(int x, int y);
	bool IsCounter(int x, int y);
	int GetTerrainTag(int x, int y);
	int CheckEvent(int x, int y);
	void StartScroll(int direction, int distance, int speed);
	bool IsScrolling();
	void StartFogToneChange(Tone tone, int duration);
	void StartFogOpacityChange(int opacity, int duration);
	void Update();
	
	std::string chipset_name;
	int animation_speed;
	int animation_type;
	std::string panorama_name;
	int panorama_hue;
	std::string fog_name;
	int fog_hue;
	int fog_opacity;
	int fog_blend_type;
	double fog_zoom;
	int fog_sx;
	int fog_sy;
	std::string battleback_name;
	int display_x;
	int display_y;
	bool need_refresh;
	std::vector<unsigned char> passages_down;
	std::vector<unsigned char> passages_up;
	std::vector<short> terrain_tags;
	std::vector<Game_Event*> events; // TODO: Implement Game_Event
	int fog_ox;
	int fog_oy;
	Tone fog_tone;

protected:
	RPG::Map map;
	int map_id;
	int scroll_direction;
	int scroll_rest;
	int scroll_speed;
	Tone fog_tone_target;
	int fog_tone_duration;
	double fog_opacity_target;
	int fog_opacity_duration;
};

#endif
