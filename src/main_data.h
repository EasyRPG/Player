#ifndef __main_data__
#define __main_data__

#include <string>

#include "scene.h"

#include "game_system.h"
#include "game_temp.h"
#include "game_system.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_screen.h"
#include "game_actors.h"
#include "game_party.h"
#include "game_troop.h"
#include "game_map.h"
#include "game_player.h"
#include "actor.h"
#include "skill.h"
#include "item.h"
#include "enemy.h"
#include "troop.h"
//#include "rpg_actor.h"
//#include "rpg_skill.h"
/*#include "rpg_object.h"
#include "rpg_enemy.h"
#include "rpg_troop.h"
#include "rpg_attribute.h"
#include "rpg_state.h"
#include "rpg_animation.h"
#include "rpg_terrain.h"
#include "rpg_chipset.h"
#include "rpg_glossary.h"
#include "rpg_system.h"
#include "rpg_common_event.h"
#include "rpg_map.h"
#include "rpg_area.h"*/

namespace Main_Data {

	// Scene Data
	Scene *scene;
	std::string scene_type;
	
	// Dynamic Game Data
	Game_System *game_system;
	Game_Temp *game_temp;
    Game_Switches *game_switches;
    Game_Variables *game_variables;
    Game_Screen *game_screen;
    Game_Actors *game_actors;
    Game_Party *game_party;
    Game_Troop *game_troop;
    Game_Map *game_map;
    Game_Player *game_player;
	
	// Database Data (ldb)
	std::vector<RPG::Actor*> data_actors;
    std::vector<RPG::Skill*> data_skills;
    std::vector<RPG::Item*> data_objects;
    std::vector<RPG::Enemy*> data_enemies;
    std::vector<RPG::Troop*> data_troops;
	/*std::vector<RPG::Attribute*> data_attributes;
    std::vector<RPG::State*> data_states;
    std::vector<RPG::Animation*> data_animations;
	std::vector<RPG::Terrain*> data_terrains;
    std::vector<RPG::ChipSet*> data_chipsets;
	std::vector<RPG::Common_Event*> data_common_events;
	RPG::Glossary* data_words;
    RPG::System* data_system;
	
	// Maps and Areas Data (lmu)
	std::vector<RPG::Map> *data_maps;
	std::vector<RPG::Area> *data_areas;*/
};
#endif // __main_data__
