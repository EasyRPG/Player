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

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Global Variables
////////////////////////////////////////////////////////////
namespace Main_Data {
    // Scene Data
    Scene *scene;
    unsigned char scene_type;
    
    // Dynamic Game Data
    Game_System *game_system = NULL;
    Game_Temp *game_temp = NULL;
    Game_Switches *game_switches = NULL;
    Game_Variables *game_variables = NULL;
    Game_Screen *game_screen = NULL;
    Game_Actors *game_actors = NULL;
    Game_Party *game_party = NULL;
    Game_Troop *game_troop = NULL;
    Game_Map *game_map = NULL;
    Game_Player *game_player = NULL;
    
    // Database Data (ldb)
    std::vector<RPG::Actor*> data_actors;
    std::vector<RPG::Skill*> data_skills;
    std::vector<RPG::Item*> data_objects;
    std::vector<RPG::Enemy*> data_enemies;
    std::vector<RPG::Troop*> data_troops;
    std::vector<RPG::Terrain*> data_terrains;
    std::vector<RPG::Attribute*> data_attributes;
    std::vector<RPG::State*> data_states;
    //std::vector<RPG::Animation*> data_animations;
    //std::vector<RPG::ChipSet*> data_chipsets;
    //std::vector<RPG::Common_Event*> data_common_events;
    RPG::Glossary* data_words = NULL;
    RPG::System* data_system = NULL;
    
    // Maps and Areas Data (lmu)
    //std::vector<RPG::Map> *data_maps;
    //std::vector<RPG::Area> *data_areas;
}

void Main_Data::Cleanup() {
    delete game_system;
    delete game_temp;
    delete game_switches;
    delete game_variables;
    delete game_screen;
    delete game_actors;
    delete game_party;
    delete game_troop;
    delete game_map;
    delete game_player;

    for (unsigned int i = 0; i < data_actors.size(); i++) delete data_actors[i];
    data_actors.clear(); // Get rid of pointers that point to nowhere safe

    for (unsigned int i = 0; i < data_skills.size(); i++) delete data_skills[i];
    data_skills.clear();

    for (unsigned int i = 0; i < data_objects.size(); i++) delete data_objects[i];
    data_objects.clear();

    for (unsigned int i = 0; i < data_enemies.size(); i++) delete data_enemies[i];
    data_enemies.clear();

    for (unsigned int i = 0; i < data_troops.size(); i++) delete data_troops[i];
    data_troops.clear();

    for (unsigned int i = 0; i < data_terrains.size(); i++) delete data_terrains[i];
    data_terrains.clear();

    for (unsigned int i = 0; i < data_attributes.size(); i++) delete data_attributes[i];
    data_attributes.clear();

    for (unsigned int i = 0; i < data_states.size(); i++) delete data_states[i];
    data_states.clear();

    delete data_words;
    delete data_system;
}
