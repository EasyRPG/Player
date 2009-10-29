#include "skill.h"

Skill::Skill() {
    id = 0;
    name = "";
    icon_name = "";
    description = "";
    text = "";
    text2 = "";
    failure_msg = 0;
    spell_type = 0;
    switch_on = 0;
    scope = 0;
    occasion = 1;
    animation1_id = 0;
    animation2_id = 0;
    menu_se = NULL; // FIXME: Should allocate some memory
    field_usage = false;
    combat_usage = false;
    affect_hp = true;
    affect_mp = false;
    affect_str = false;
    affect_pdef = false;
    //affect_mdef = false; Not available in rm 2k/2k3
    affect_int = false;
    affect_agi = false;
    affect_resistance = false;
    absorb_dmg = false;
    ignore_def = false;
    common_event_id = 0; 
    sp_cost = 0;
    power = 0;
    atk_f = 0;
    eva_f = 0;
    str_f = 0;
    dex_f = 0;
    agi_f = 0;
    int_f = 3;
    hit = 100;
    pdef_f = 0;
    mdef_f = 100;
    variance = 4;   
}
