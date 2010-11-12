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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "game_map.h"
#include "lmu_reader.h"
#include "game_event.h"

////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////
#define min(a, b)	(((a) < (b)) ? (a) : (b))
#define max(a, b)	(((a) > (b)) ? (a) : (b))

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Game_Map::Game_Map() {
	panorama_hue = 0;
	fog_hue = 0;
	fog_opacity = 0;
	fog_blend_type = 0;
	fog_zoom = 0.0;
	fog_sx = 0;
	fog_sy = 0;
	need_refresh = false;
	fog_ox = 0;
	fog_oy = 0;
	map_id = 0;
	display_x = 0;
	display_y = 0;
	fog_opacity_duration = 0;
	fog_opacity_target = 0;
	fog_tone_duration = 0;
	animation_speed = 0;
	animation_type = 0;
	scroll_direction = 0;
	scroll_rest = 0;
	scroll_speed = 0;
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_Map::~Game_Map() {
	unsigned int i;
	for (i = 0; i < events.size(); i++) {
		delete events[i];
	}
}

////////////////////////////////////////////////////////////
/// Setup
////////////////////////////////////////////////////////////
void Game_Map::Setup(int _id) {
	map_id = _id;
	char file[12];
	sprintf(file, "Map%04d.lmu", map_id);
	map = LMU_Reader::LoadMap(file);

	RPG::Chipset chipset = Main_Data::data_chipsets[map.chipset_id - 1];
	chipset_name = chipset.chipset_name;
	passages_down = chipset.passable_data_lower;
	passages_up = chipset.passable_data_lower;
	terrain_tags = chipset.terrain_data;
	animation_speed = chipset.animation_speed;
	animation_type = chipset.animation_type;
	display_x = 0;
	display_y = 0;
	need_refresh = false;
	/*events.clear();
	for i in @map.events.keys
		@events[i] = Game_Event.new(@map_id, @map.events[i])
	end
	@common_events.clear();
	for i in 1...$data_common_events.size
		@common_events[i] = Game_CommonEvent.new(i)
	end*/
	fog_ox = 0;
	fog_oy = 0;
	fog_tone = Tone(0, 0, 0, 0);
	fog_tone_target = Tone(0, 0, 0, 0);
	fog_tone_duration = 0;
	fog_opacity_duration = 0;
	fog_opacity_target = 0;
	scroll_direction = 2;
	scroll_rest = 0;
	scroll_speed = 4;
}

////////////////////////////////////////////////////////////
/// Get Map Id
////////////////////////////////////////////////////////////
int Game_Map::GetMapId() const {
	return map_id;
}

////////////////////////////////////////////////////////////
/// Get Width
////////////////////////////////////////////////////////////
int Game_Map::GetWidth() const {
	return map.width;
}

////////////////////////////////////////////////////////////
/// Get Height
////////////////////////////////////////////////////////////
int Game_Map::GetHeight() const {
	return map.height;
}

////////////////////////////////////////////////////////////
/// Get Encounter List
////////////////////////////////////////////////////////////
std::vector<int> Game_Map::GetEncounterList() {
	return std::vector<int>();
}

////////////////////////////////////////////////////////////
/// Get Encounter Step
////////////////////////////////////////////////////////////
int Game_Map::GetEncounterStep() {
	return 0;
}

////////////////////////////////////////////////////////////
/// Get Map Data Down
////////////////////////////////////////////////////////////
std::vector<short> Game_Map::GetMapDataDown() {
	return map.lower_layer;
}

////////////////////////////////////////////////////////////
/// Get Map Data Up
////////////////////////////////////////////////////////////
std::vector<short> Game_Map::GetMapDataUp() {
	return map.upper_layer;
}

////////////////////////////////////////////////////////////
/// Autoplay
////////////////////////////////////////////////////////////
void Game_Map::Autoplay() {

}

////////////////////////////////////////////////////////////
/// Refresh
////////////////////////////////////////////////////////////
void Game_Map::Refresh() {
	/*if (map_id > 0) {
		for (int i = 0; i < events.size(); i++) {
			events[i]->Refresh();
		}
		for (int i = 0; i < common_events.size(); i++) {
			common_events[i]->Refresh();
		}
	}*/
	need_refresh = false;
}

////////////////////////////////////////////////////////////
/// Scroll Down
////////////////////////////////////////////////////////////
void Game_Map::ScrollDown(int distance) {
	display_y = min(display_y + distance, (map.height - 15) * 128);
}

////////////////////////////////////////////////////////////
/// Scroll Left
////////////////////////////////////////////////////////////
void Game_Map::ScrollLeft(int distance) {
	display_x = max(display_x - distance, 0);
}

////////////////////////////////////////////////////////////
/// Scroll Right
////////////////////////////////////////////////////////////
void Game_Map::ScrollRight(int distance) {
	display_x = min(display_x + distance, (map.width - 20) * 128);
}

////////////////////////////////////////////////////////////
/// Scroll Up
////////////////////////////////////////////////////////////
void Game_Map::ScrollUp(int distance) {
	display_y = max(display_y - distance, 0);
}

////////////////////////////////////////////////////////////
/// Is Valid
////////////////////////////////////////////////////////////
bool Game_Map::IsValid(int x, int y) {
	return (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight());
}

////////////////////////////////////////////////////////////
/// Is Passable
////////////////////////////////////////////////////////////
/*bool Game_Map::IsPassable(int x, int y, int d, Game_Event* self_event = NULL) {

}*/

////////////////////////////////////////////////////////////
/// Is Bush
////////////////////////////////////////////////////////////
bool Game_Map::IsBush(int x, int y) {
	return false;
}

////////////////////////////////////////////////////////////
/// Is Counter
////////////////////////////////////////////////////////////
bool Game_Map::IsCounter(int x, int y) {
	return false;
}

////////////////////////////////////////////////////////////
/// Get Terrain Tag
////////////////////////////////////////////////////////////
int Game_Map::GetTerrainTag(int x, int y) {
	return 0;
}

////////////////////////////////////////////////////////////
/// Check Event
////////////////////////////////////////////////////////////
int Game_Map::CheckEvent(int x, int y) {
	/*for (int i = 0; i < events.size(); i++) {
		if (events[i]->x == x && events[i]->y == y) {
			return events[i]->id;
		}
	}*/
	return 0;
}

////////////////////////////////////////////////////////////
/// Start Scroll
////////////////////////////////////////////////////////////
void Game_Map::StartScroll(int direction, int distance, int speed) {
	scroll_direction = direction;
	scroll_rest = distance * 128;
	scroll_speed = speed;
}

////////////////////////////////////////////////////////////
/// Is Scrolling
////////////////////////////////////////////////////////////
bool Game_Map::IsScrolling() {
	return scroll_rest > 0;
}

////////////////////////////////////////////////////////////
/// Start Fog Tone Change
////////////////////////////////////////////////////////////
void Game_Map::StartFogToneChange(Tone tone, int duration) {
	fog_tone_target = Tone(tone);
	fog_tone_duration = duration;
	if (fog_tone_duration == 0) {
		fog_tone = Tone(tone);
	}
}

////////////////////////////////////////////////////////////
/// Start Fog Opacity Change
////////////////////////////////////////////////////////////
void Game_Map::StartFogOpacityChange(int opacity, int duration) {
	fog_opacity_target = opacity * 1.0;
	fog_opacity_duration = duration;
	if (fog_opacity_duration == 0) {
		fog_opacity = (int)fog_opacity_target;
	}
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Game_Map::Update() {
	if (need_refresh) Refresh();
	if (scroll_rest > 0) {
		int distance = 2 ^ scroll_speed;
		switch (scroll_direction) {
			case 2:
				ScrollDown(distance);
				break;
			case 4:
				ScrollLeft(distance);
				break;
			case 6:
				ScrollRight(distance);
				break;
			case 8:
				ScrollUp(distance);
				break;
		}
		scroll_rest -= distance;
	}
	
	/*for (int i = 0; i < events.size(); i++) {
		Main_Data::game_map->events[i].Update();
	}

	for (int i = 0; i < common_events.size(); i++) {
		common_events[i].Update();
	}

	fog_ox -= fog_sx / 8.0;
	fog_oy -= fog_sy / 8.0;
	if (fog_tone_duration >= 1) {
		int d = fog_tone_duration;
		Tone target = fog_tone_target;
		fog_tone.red = (fog_tone.red * (d - 1) + target.red) / d;
		fog_tone.green = (fog_tone.green * (d - 1) + target.green) / d;
		fog_tone.blue = (fog_tone.blue * (d - 1) + target.blue) / d;
		fog_tone.gray = (fog_tone.gray * (d - 1) + target.gray) / d;
		fog_tone_duration -= 1;
	}
	if (fog_opacity_duration >= 1)
		int d = fog_opacity_duration;
		fog_opacity = (fog_opacity * (d - 1) + fog_opacity_target) / d;
		fog_opacity_duration -= 1;
	}*/
}
