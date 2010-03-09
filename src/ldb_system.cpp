#include "ldb_reader.h"
#include "rpg_system.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::systemChunk(FILE * Stream)//movimientos de la pagina
{
    short dat;
    Main_Data::data_system = new RPG::System();
    do {
        ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
        if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
        switch (ChunkInfo.ID) { // tipo de la primera dimencion
        /*case LDB_ID://0x0A,
            Main_Data::data_system->intLDB_ID = ReadCompressedInteger(Stream);
            break;*/
        case Skiff_graphic://0x0B,
            Main_Data::data_system->boat_name = ReadString(Stream, ChunkInfo.Length);
            break;
        case Boat_graphic://0x0C,
            Main_Data::data_system->ship_name = ReadString(Stream, ChunkInfo.Length);
            break;
        case Airship_graphic://0x0D,
            Main_Data::data_system->airship_name = ReadString(Stream, ChunkInfo.Length);
            break;
        case Skiff_index://0x0E,
            Main_Data::data_system->boat_index = ReadCompressedInteger(Stream);
            break;
        case Boat_index://0x0F,
            Main_Data::data_system->ship_index = ReadCompressedInteger(Stream);
            break;
        case Airship_index://0x10,
            Main_Data::data_system->airship_index = ReadCompressedInteger(Stream);
            break;
        case Title_graphic://0x11,
            Main_Data::data_system->title_name = ReadString(Stream, ChunkInfo.Length);
            break;
        case Game_Over_graphic://0x12,
            Main_Data::data_system->gameover_name = ReadString(Stream, ChunkInfo.Length);
            break;
        case System_graphic://0x13,
            Main_Data::data_system->windowskin_name = ReadString(Stream, ChunkInfo.Length);
            break;
        case System_graphic_2://0x14,
            Main_Data::data_system->windowskin2_name = ReadString(Stream, ChunkInfo.Length);
            break;
        /*case Heroes_in_starting://0x15,
            Main_Data::data_system->Heroes_in_starting = ReadCompressedInteger(Stream);
            break;*/
        case Starting_party://0x16,
            while (ChunkInfo.Length--) {
                bool return_value;
                return_value = fread(&dat, 2, 1, Stream);
                Main_Data::data_system->party_members.push_back(dat);
                ChunkInfo.Length--;
            }
            break;
        /*case Num_Comadns_order://0x1A,
            Main_Data::data_system->intNum_Comadns_order = ReadCompressedInteger(Stream);
            break;
        case Comadns_order://0x1B,
            while (ChunkInfo.Length--) {
                bool return_value;
                return_value = fread(&dat, 2, 1, Stream);
                System->vc_sh_Comadns_order.push_back(dat);
                ChunkInfo.Length--;
            }
            break;*/
        case Title_music://0x1F,
            musicChunk(Stream, Main_Data::data_system->title_music);//0x1F,
            break;
        case Battle_music://0x20,
            musicChunk(Stream, Main_Data::data_system->battle_music);//0x20,
            break;
        case Battle_end_music://0x21,
            musicChunk(Stream, Main_Data::data_system->battle_end_music);//0x21,
            break;
        case Inn_music://0x21,
            musicChunk(Stream, Main_Data::data_system->inn_music);//0x22,
            break;
        case Skiff_music://0x21,
            musicChunk(Stream, Main_Data::data_system->boat_music);//0x23,
            break;
        case Boat_music://0x21,
            musicChunk(Stream, Main_Data::data_system->ship_music);//0x24,
            break;
        case Airship_music://0x21,
            musicChunk(Stream, Main_Data::data_system->airship_music);//0x25,
            break;
        case Game_Over_music://0x21,
            musicChunk(Stream, Main_Data::data_system->gameover_music);//0x26,
            break;
        case Cursor_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->cursor_se);//0x29,
            break;
        case Accept_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->decision_se);//0x2A,
            break;
        case Cancel_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->cancel_se);//0x2B,
            break;
        case Illegal_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->buzzer_se);//0x2C,
            break;
        case Battle_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->battle_start_se);//0x2D,
            break;
        case Escape_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->escape_se);//0x2E,
            break;
        case Enemy_attack_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->enemy_attack_se);//0x2F,
            break;
        case Enemy_damaged_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->enemy_damaged_se);//0x30,
            break;
        case Ally_damaged_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->actor_damaged_se);//0x31,
            break;
        case Evasion_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->evasion_se);//0x32,
            break;
        case Enemy_dead_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->enemy_collapse_se);//0x33,
            break;
        case Item_use_SFX://0x21,
            soundChunk(Stream, Main_Data::data_system->item_use_se);//0x34,
            break;
        case Map_exit_transition://0x3D,
            Main_Data::data_system->map_exit_transition = ReadCompressedInteger(Stream);
            break;
        case Map_enter_transition://0x3E,
            Main_Data::data_system->map_enter_transition = ReadCompressedInteger(Stream);
            break;
        case Battle_start_fadeout://0x3F,
            Main_Data::data_system->battle_start_fadeout = ReadCompressedInteger(Stream);
            break;
        case Battle_start_fadein://0x40,
            Main_Data::data_system->battle_start_fadein = ReadCompressedInteger(Stream);
            break;
        case Battle_end_fadeout://0x41,
            Main_Data::data_system->battle_end_fadeout = ReadCompressedInteger(Stream);
            break;
        case Battle_end_fadein://0x42,
            Main_Data::data_system->battle_end_fadein = ReadCompressedInteger(Stream);
            break;
        /*case Message_background://0x47,
            Main_Data::data_system->Message_background = ReadCompressedInteger(Stream);
            break;
        case Font_id://0x48,
            Main_Data::data_system->Font = ReadCompressedInteger(Stream);
            break;
        case Selected_condition://0x51,
            Main_Data::data_system->Selected_condition = ReadCompressedInteger(Stream);
            break;
        case Selected_hero://0x52,
            Main_Data::data_system->Selected_hero = ReadCompressedInteger(Stream);
            break;*/
        case Battle_test_BG://0x54,
            Main_Data::data_system->battleback_name = ReadString(Stream, ChunkInfo.Length);
            break;
        /*case Battle_test_data://0x55
            Main_Data::data_system->test_battlers=Batletest(Stream);
            break;*/
        /*case Times_saved://0x41,
            Main_Data::data_system->Times_saved = ReadCompressedInteger(Stream);
            break;
        case Show_frame://0x42,
            Main_Data::data_system->Show_frame = ReadCompressedInteger(Stream);
            break;
        case In_battle_anim://0x47,
            Main_Data::data_system->In_battle_anim = ReadCompressedInteger(Stream);
            break;*/
        case CHUNK_LDB_END_OF_BLOCK:
            break;
        default:
            while (ChunkInfo.Length--) {
                bool return_value;
                return_value = fread(&Void, 1, 1, Stream);
            }
            break;
        }
    } while (ChunkInfo.ID!=0);
}
