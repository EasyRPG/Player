#include "rpg_state.h"

RPG::State::State() {
    id = 0;
    name = "";
    battle_only = 0;
    color = 0;
    priority = 0;
    restriction = 0; //Sets restrictions (0: none, 1: can't use magic, 2: always attack enemies, 3: always attack allies, 4: can't move).
    A_chance = 0;
    B_chance = 0;
    C_chance = 0;
    D_chance = 0;
    E_chance = 0;
    hold_turn = 0;
    auto_release_prob = 0;
    shock_release_prob = 0;
    type_change = 0; //0: reducir a la mitad 1: doble 2: No cambiar
    atk_mod = 0;
    pdef_mod = 0;
    int_mod = 0;
    agi_mod = 0;
    rate_change = 0;
    prevent_skill_use = 0;
    min_skill_lvl = 0;
    prevent_magic_use = 0;
    min_magic_lvl = 0;
    ally_enter_state = "";
    enemy_enter_state = "";
    already_in_state = "";
    affected_by_state = "";
    status_recovered = "";
    hp_loss = 0;
    hp_loss_value = 0;
    hp_map_loss = 0;
    hp_map_steps = 0;
    mp_loss = 0;
    mp_loss_value = 0;
    mp_map_loss = 0;
    mp_map_steps = 0;
}