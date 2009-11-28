#include "main_data.h"

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


    void cleanup() {
        int s = 0, i;

        
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

        s = data_actors.size();
        for (i = 0; i < s; ++i) {
            delete data_actors[i];
        }
        /* Get rid of pointers that point to nowhere safe */
        data_actors.clear();

        s = data_skills.size();
        for (i = 0; i < s; ++i) {
            delete data_skills[i];
        }
        data_skills.clear();

        s = data_objects.size();
        for (i = 0; i < s; ++i) {
            delete data_objects[i];
        }
        data_objects.clear();

        s = data_enemies.size();
        for (i = 0; i < s; ++i) {
            delete data_enemies[i];
        }
        data_enemies.clear();

        s = data_troops.size();
        for (i = 0; i < s; ++i) {
            delete data_troops[i];
        }
        data_troops.clear();

        s = data_terrains.size();
        for (i = 0; i < s; ++i) {
            delete data_terrains[i];
        }
        data_terrains.clear();

        s = data_attributes.size();
        for (i = 0; i < s; ++i) {
            delete data_attributes[i];
        }
        data_attributes.clear();

        s = data_states.size();
        for (i = 0; i < s; ++i) {
            delete data_states[i];
        }
        data_states.clear();

        delete data_words;

        delete data_system;
    }
}
