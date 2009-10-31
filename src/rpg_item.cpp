#include "rpg_item.h"

RPG::Item::Item() {
    id = 0;
    name = "";
    description = "";
    type = COMMON;
    price = 0;
    n_uses = 1;
    atk_change = 0;
    pdef_change = 0;
    int_change = 0;
    agi_change = 0;
    both_hands = false;
    mp_cost = 0;
    hit = 90;
    crit_hit = 0;
    animation_id = 0;
    preventive = false;
    double_attack = false;
    attack_all = false;
    ignore_evasion = false;
    prevent_crits = false;
    raise_evasion = false;
    half_mp = false;
    no_terrain_dmg = false;
    heals_party = false;
    recover_hp = 0;
    recover_hp_rate = 0;
    recover_mp = 0;
    recover_mp_rate = 0;
    field_only = false;
    dead_only = false;
    max_hp_mod = 0;
    max_mp_mod = 0;
    str_mod = 0;
    pdef_mod = 0;
    int_mod = 0;
    agi_mod = 0;
    use_msg = true;
    switch_on = 0;
    use_on_field = true;
    use_in_battle = false;
    chance_cond = 0;
    
}
