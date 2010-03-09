//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

#ifndef _MAIN_DATA_H_
#define _MAIN_DATA_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>

#include "scene.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_system.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_screen.h"
#include "game_battler.h"
#include "game_actor.h"
#include "game_actors.h"
#include "game_party.h"
#include "game_troop.h"
#include "game_map.h"
#include "game_player.h"
#include "rpg_actor.h"
#include "rpg_skill.h"
#include "rpg_item.h"
#include "rpg_enemy.h"
#include "rpg_troop.h"
#include "rpg_enemy.h"
#include "rpg_terrain.h"
#include "rpg_attribute.h"
#include "rpg_state.h"
//#include "rpg_animation.h"
//#include "rpg_chipset.h"
#include "rpg_glossary.h"
#include "rpg_system.h"
//#include "rpg_common_event.h"
//#include "rpg_map.h"
//#include "rpg_area.h"

////////////////////////////////////////////////////////////
/// Main Data namespace
////////////////////////////////////////////////////////////
namespace Main_Data {
    // Scene Data
    extern Scene *scene;
    extern unsigned char scene_type;
    
    // Dynamic Game Data
    extern Game_System* game_system;
    extern Game_Temp* game_temp;
    extern Game_Switches* game_switches;
    extern Game_Variables* game_variables;
    extern Game_Screen* game_screen;
    extern Game_Actors* game_actors;
    extern Game_Party* game_party;
    extern Game_Troop* game_troop;
    extern Game_Map* game_map;
    extern Game_Player* game_player;
    
    // Database Data (ldb)
    extern std::vector<RPG::Actor*> data_actors;
    extern std::vector<RPG::Skill*> data_skills;
    extern std::vector<RPG::Item*> data_objects;
    extern std::vector<RPG::Enemy*> data_enemies;
    extern std::vector<RPG::Troop*> data_troops;
    extern std::vector<RPG::Terrain*> data_terrains;
    extern std::vector<RPG::Attribute*> data_attributes;
    extern std::vector<RPG::State*> data_states;
    //extern std::vector<RPG::Animation*> data_animations;
    //extern std::vector<RPG::ChipSet*> data_chipsets;
    //extern std::vector<RPG::Common_Event*> data_common_events;
    extern RPG::Glossary* data_words;
    extern RPG::System* data_system;

    // Maps and Areas Data (lmu)
    //extern std::vector<RPG::Map> *data_maps;
    //extern std::vector<RPG::Area> *data_areas;

    void Cleanup();
}

#endif
