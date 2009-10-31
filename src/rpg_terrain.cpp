#include "rpg_terrain.h"

RPG::Terrain::Terrain() {
    id = 0;
    name = "";
    damage = 0;
    encounter_rate = 0;
    battle_bkg_name = "";
    skiff_may_pass = false;
    boat_may_pass = false;
    airship_may_pass = true;
    airship_may_land = true;
    hero_opacity = 0;
}