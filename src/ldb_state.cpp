#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::statesChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    RPG::State *state;
    
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        state = new RPG::State();
        state->id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case StatesChunk_Name:
                state->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Length://0x02,
                state->battle_only = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Color://0x03,
                state->color = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Priority://0x04,
                state->priority = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Limitation://0x05,
                state->restriction = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_A_chance://0x0B,
                state->A_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_B_chance://0x0C,
                state->B_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_C_chance://0x0D,
                state->C_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_D_chance://0x0E,
                state->D_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_E_chance://0x0F,
                state->E_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Turnsforhealing://0x15,
                state->hold_turn = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Healperturn://0x16,
                state->auto_release_prob = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Healonshock://0x17,
                state->shock_release_prob = ReadCompressedInteger(Stream);
                break;
            case 0x1E: //0: reducir a la mitad 1: doble 2: No cambiar
                state->type_change = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveAttack://0x1F,
                state->atk_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveDefense://0x20,
                state->pdef_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveMind://0x21,
                state->int_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveAgility://0x22,
                state->agi_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Hitratechange://0x23,
                state->rate_change = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Preventskilluse://0x29,
                state->prevent_skill_use = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Minimumskilllevel://0x2A,
                state->min_skill_lvl = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Preventmagicuse://0x2B,
                state->prevent_magic_use = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Minimummindlevel://0x2C,
                state->min_magic_lvl = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Allyenterstate://0x33,
                state->ally_enter_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Enemyentersstate://0x34,
                state->enemy_enter_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Alreadyinstate://0x35,
                state->already_in_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Affectedbystate://0x36,
                state->affected_by_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Statusrecovered://0x37,
                state->status_recovered = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_HPloss://0x3D,
                state->hp_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HPlossvalue://0x3E,
                state->hp_loss_value = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HPmaploss://0x3F,
                state->hp_map_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HPmapsteps://0x40,
                state->hp_map_steps = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPloss://0x41,
                state->mp_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPlossvalue://0x42,
                state->mp_loss_value = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPmaploss://0x43,
                state->mp_map_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPmapsteps://0x44
                state->mp_map_steps = ReadCompressedInteger(Stream);
                break;
            case CHUNK_LDB_END_OF_BLOCK:
                break;
            default:
                // saltate un pedazo del tamaño de la longitud
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                }
                break;
            }
        } while (ChunkInfo.ID!=0);
        datareaded++;
        Main_Data::data_states.push_back(state);
    }
}
