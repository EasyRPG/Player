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

#ifndef _MAIN_DATA_H_
#define _MAIN_DATA_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>

#include "scene.h"
#include "rpg_actor.h"
#include "rpg_skill.h"
#include "rpg_item.h"
#include "rpg_enemy.h"
#include "rpg_troop.h"
#include "rpg_attribute.h"
#include "rpg_state.h"
#include "rpg_terrain.h"
#include "rpg_animation.h"
#include "rpg_chipset.h"
#include "rpg_terms.h"
#include "rpg_system.h"
#include "rpg_commonevent.h"
#include "rpg_class.h"
#include "rpg_battlecommand.h"
#include "rpg_battleranimation.h"
#include "rpg_sound.h"
#include "rpg_music.h"

#include "rpg_eventcommand.h"

#include "rpg_treemap.h"
#include "rpg_map.h"
//#include "rpg_area.h"

////////////////////////////////////////////////////////////
/// Main Data namespace
////////////////////////////////////////////////////////////
class Game_Party;
class Game_Player;
class Game_Screen;
class Game_Troop;
class Game_Message;

namespace Main_Data {
	// Scene Data
	extern Scene* scene;
	extern Scene* old_scene;
	extern unsigned char scene_type;
	
	// Dynamic Game Data
	extern Game_Screen* game_screen;
	extern Game_Party* game_party;
	extern Game_Player* game_player;
	extern Game_Troop* game_troop;
	extern Game_Player* game_player;
	extern Game_Message* game_message;
	
	// Database Data (ldb)
	extern std::vector<RPG::Actor> data_actors;
	extern std::vector<RPG::Skill> data_skills;
	extern std::vector<RPG::Item> data_items;
	extern std::vector<RPG::Enemy> data_enemies;
	extern std::vector<RPG::Troop> data_troops;
	extern std::vector<RPG::Terrain> data_terrains;
	extern std::vector<RPG::Attribute> data_attributes;
	extern std::vector<RPG::State> data_states;
	extern std::vector<RPG::Animation> data_animations;
	extern std::vector<RPG::Chipset> data_chipsets;
	extern std::vector<RPG::CommonEvent> data_commonevents;
	extern std::vector<RPG::BattleCommand> data_battlecommands;
	extern std::vector<RPG::Class> data_classes;
	extern std::vector<RPG::BattlerAnimation> data_battleranimations;
	extern RPG::Terms data_terms;
	extern RPG::System data_system;
	extern std::vector<std::string> data_switches;
	extern std::vector<std::string> data_variables;

	// TreeMap (lmt)
	extern RPG::TreeMap data_treemap;

	// Maps and Areas Data (lmu)
	//extern std::vector<RPG::Map> data_maps;
	//extern std::vector<RPG::Area> data_areas;

	void Cleanup();
}

#endif
