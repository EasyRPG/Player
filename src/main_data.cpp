#include "main_data.h"

namespace Main_Data {
    // Scene Data
	Scene *scene;
    unsigned char scene_type;
	
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
    std::vector<RPG::Terrain*> data_terrains;
    std::vector<RPG::Attribute*> data_attributes;
    std::vector<RPG::State*> data_states;
	//std::vector<RPG::Animation*> data_animations;
    //std::vector<RPG::ChipSet*> data_chipsets;
	//std::vector<RPG::Common_Event*> data_common_events;
	RPG::Glossary* data_words;
    RPG::System* data_system;
	
	// Maps and Areas Data (lmu)
	//std::vector<RPG::Map> *data_maps;
	//std::vector<RPG::Area> *data_areas;
}
