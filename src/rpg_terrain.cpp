#include "rpg_terrain.h"

RPG::Terrain::Terrain() {
    id = 0;
    name = "";
    damage = 0;
    encounter_rate = 0;
    battle_background = "";
    ship_pass = false;
    boat_pass = false;
    airship_pass = true;
    airship_land = true;
    chara_opacity = 0;
}