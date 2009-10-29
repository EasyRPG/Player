#include "actor.h"

Actor::Actor() {
   // register int i;
    id = 0;
    name = "";
    title_name = "";
    class_id = 1;
    initial_level = 1;
    final_level = 50;
    exp_basis = 30;
    exp_inflation = 30;
    critical_hit = 1;
    critical_chance = 30;
    graphic_index = 0;
    face_name = "";
    face_index = 0;
    character_name = "";
    character_hue = 0;
    transparent = false;
    battler_name = "";
    battler_hue = 0;
   /* for (i = 1; i <= 99; ++i) {
        parameters[0][i] = 500+i*50;
        parameters[1][i] = 500+i*50;
        parameters[2][i] = 50+i*5;
        parameters[3][i] = 50+i*5;
        parameters[4][i] = 50+i*5;
        parameters[5][i] = 50+i*5;
    }*/
    double_weapon = false;
    lock_equipment = false;
    ia_control = false;
    high_defense = false;
    exp_base_line = 10;
    exp_additional = 50;
    exp_correction = 0;
    weapon_id = 0;
    armor1_id = 0;
    armor2_id = 0;
    armor3_id = 0;
    armor4_id = 0;
    weapon_fix = false;
    armor1_fix = false;
    armor2_fix = false;
    armor3_fix = false;
    armor4_fix = false;
    skill_rename = false;
}
