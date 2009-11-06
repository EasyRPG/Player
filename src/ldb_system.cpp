#include "ldb_system.h"

void LDB_reader::systemChunk(FILE * Stream)//movimientos de la pagina
{
    short dat;
    RPG::System *system;
    system = new RPG::System();
    if (system == NULL)
    {
        std::cerr << "No memory left." << std::endl;
        exit(-1);
    }
    System->Heroes_in_starting =1;//default
    do {
        ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
        if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
        switch (ChunkInfo.ID) { // tipo de la primera dimencion
        case LDB_ID://0x0A,
            System->intLDB_ID = ReadCompressedInteger(Stream);
            break;
        case Skiff_graphic://0x0B,
            System->Skiff_graphic = ReadString(Stream, ChunkInfo.Length);
            break;
        case Boat_graphic://0x0C,
            System->Boat_graphic = ReadString(Stream, ChunkInfo.Length);
            break;
        case Airship_graphic://0x0D,
            System->Airship_graphic = ReadString(Stream, ChunkInfo.Length);
            break;
        case Skiff_index://0x0E,
            System->Skiff_index = ReadCompressedInteger(Stream);
            break;
        case Boat_index://0x0F,
            System->Boat_index = ReadCompressedInteger(Stream);
            break;
        case Airship_index://0x10,
            System->Airship_index = ReadCompressedInteger(Stream);
            break;
        case Title_graphic://0x11,
            System->Title_graphic = ReadString(Stream, ChunkInfo.Length);
            break;
        case Game_Over_graphic://0x12,
            System->Game_Over_graphic = ReadString(Stream, ChunkInfo.Length);
            break;
        case System_graphic://0x13,
            System->System_graphic = ReadString(Stream, ChunkInfo.Length);
            break;
        case System_graphic_2://0x14,
            System->System_graphic_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Heroes_in_starting://0x15,
            System->Heroes_in_starting = ReadCompressedInteger(Stream);
            break;
        case Starting_party://0x16,
            while (ChunkInfo.Length--) {
                bool return_value;
                return_value = fread(&dat, 2, 1, Stream);
                System->vc_sh_Starting_party.push_back(dat);
                ChunkInfo.Length--;
            }
            break;
        case Num_Comadns_order://0x1A,
            System->intNum_Comadns_order = ReadCompressedInteger(Stream);
            break;
        case Comadns_order://0x1B,
            while (ChunkInfo.Length--) {
                bool return_value;
                return_value = fread(&dat, 2, 1, Stream);
                System->vc_sh_Comadns_order.push_back(dat);
                ChunkInfo.Length--;
            }
            break;
        case Title_music://0x1F,
            System->Title_music=musicChunk(Stream);//0x1F,
            break;
        case Battle_music://0x20,
            System->Battle_music=musicChunk(Stream);//0x20,
            break;
        case Battle_end_music://0x21,
            System->Battle_end_music=musicChunk(Stream);//0x21,
            break;
        case Inn_music://0x21,
            System->Inn_music=musicChunk(Stream);//0x22,
            break;
        case Skiff_music://0x21,
            System->Skiff_music=musicChunk(Stream);//0x23,
            break;
        case Boat_music://0x21,
            System->Boat_music=musicChunk(Stream);//0x24,
            break;
        case Airship_music://0x21,
            System->Airship_music=musicChunk(Stream);//0x25,
            break;
        case Game_Over_music://0x21,
            System->Game_Over_music=musicChunk(Stream);//0x26,
            break;
        case Cursor_SFX://0x21,
            System->Cursor_SFX=soundChunk(Stream);//0x29,
            break;
        case Accept_SFX://0x21,
            System->Accept_SFX=soundChunk(Stream);//0x2A,
            break;
        case Cancel_SFX://0x21,
            System->Cancel_SFX=soundChunk(Stream);//0x2B,
            break;
        case Illegal_SFX://0x21,
            System->Illegal_SFX=soundChunk(Stream);//0x2C,
            break;
        case Battle_SFX://0x21,
            System->Battle_SFX=soundChunk(Stream);//0x2D,
            break;
        case Escape_SFX://0x21,
            System->Escape_SFX=soundChunk(Stream);//0x2E,
            break;
        case Enemy_attack_SFX://0x21,
            System->Enemy_attack_SFX=soundChunk(Stream);//0x2F,
            break;
        case Enemy_damaged_SFX://0x21,
            System->Enemy_damaged_SFX=soundChunk(Stream);//0x30,
            break;
        case Ally_damaged_SFX://0x21,
            System->Ally_damaged_SFX=soundChunk(Stream);//0x31,
            break;
        case Evasion_SFX://0x21,
            System->Evasion_SFX=soundChunk(Stream);//0x32,
            break;
        case Enemy_dead_SFX://0x21,
            System->Enemy_dead_SFX=soundChunk(Stream);//0x33,
            break;
        case Item_use_SFX://0x21,
            System->Item_use_SFX=soundChunk(Stream);//0x34,
            break;
        case Map_exit_transition://0x3D,
            System->Map_exit_transition = ReadCompressedInteger(Stream);
            break;
        case Map_enter_transition://0x3E,
            System->Map_enter_transition = ReadCompressedInteger(Stream);
            break;
        case Battle_start_fadeout://0x3F,
            System->Battle_start_fadeout = ReadCompressedInteger(Stream);
            break;
        case Battle_start_fadein://0x40,
            System->Battle_start_fadein = ReadCompressedInteger(Stream);
            break;
        case Battle_end_fadeout://0x41,
            System->Battle_end_fadeout = ReadCompressedInteger(Stream);
            break;
        case Battle_end_fadein://0x42,
            System->Battle_end_fadein = ReadCompressedInteger(Stream);
            break;
        case Message_background://0x47,
            System->Message_background = ReadCompressedInteger(Stream);
            break;
        case Font_id://0x48,
            System->Font = ReadCompressedInteger(Stream);
            break;
        case Selected_condition://0x51,
            System->Selected_condition = ReadCompressedInteger(Stream);
            break;
        case Selected_hero://0x52,
            System->Selected_hero = ReadCompressedInteger(Stream);
            break;
        case Battle_test_BG://0x54,
            System->Battle_test_BG = ReadString(Stream, ChunkInfo.Length);
            break;
        case Battle_test_data://0x55
            System->vc_Battle_test=Batletest(Stream);
            break;
        case Times_saved://0x41,
            System->Times_saved = ReadCompressedInteger(Stream);
            break;
        case Show_frame://0x42,
            System->Show_frame = ReadCompressedInteger(Stream);
            break;
        case In_battle_anim://0x47,
            System->In_battle_anim = ReadCompressedInteger(Stream);
            break;
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