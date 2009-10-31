#include "rpg_enemy.h"

RPG::Enemy::Enemy() {
    id = 0;
    name = "";
    battler_name = "";
    battler_hue = 0;
    maxhp = 0;
    maxmp = 0;
    str = 0;
    agi = 0;
    iint = 0;
    pdef = 0;
    transparent = false;
    exp = 0;
    gold = 0;
    item_id = 0;
    treasure_prob = 0;
    crit_hits = false;
    crit_hits_chance = 0;
    miss = false;
    flying = false;    
}
