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
//#include "rpg_glossary.h"
#include "rpg_system.h"
//#include "rpg_common_event.h"
//#include "rpg_map.h"
//#include "rpg_area.h"

namespace Main_Data {

	// Scene Data
	extern Scene *scene;
	extern std::string scene_type;
	
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
    //std::vector<RPG::Animation*> data_animations;
    //std::vector<RPG::ChipSet*> data_chipsets;
	//std::vector<RPG::Common_Event*> data_common_events;
	//RPG::Glossary* data_words;
    extern RPG::System* data_system;
	
	// Maps and Areas Data (lmu)
	//std::vector<RPG::Map> *data_maps;
	//std::vector<RPG::Area> *data_areas;
}
#endif // __main_data__
