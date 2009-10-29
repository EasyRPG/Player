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
#include "data_actors.h"
#include "data_skills.h"
#include "data_objects.h"
#include "data_enemies.h"
#include "data_troops.h"
#include "data_attributes.h"
#include "data_states.h"
#include "data_animations.h"
#include "data_terrains.h"
#include "data_chipsets.h"
#include "data_glossary.h"
#include "data_system.h"
#include "data_common_events.h"
#include "data_maps.h"
#include "data_areas.h"

namespace Main_Data {

	// Scene Data
	Scene *scene;
	std::string scene_type;
	
	// Dynamic Game Data (lmu)
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
	std::vector<Actor*> data_actors;
    std::vector<Skill*> data_skills;
    Data_Objects *data_objects;
    Data_Enemies *data_enemies;
    Data_Troops *data_troops;
	Data_Attributes *data_attributes;
    Data_States *data_states;
    Data_Animations *data_animations;
	Data_Terrains *data_terrains;
    Data_ChipSets *data_chipsets;
	Data_Glossary *data_words;
    Data_System *data_system;
	Data_Common_Events *data_common_events;
	
	// Maps and Areas Data
	Data_Maps *data_maps;
	Data_Areas *data_areas;
};
#endif // __main_data__
