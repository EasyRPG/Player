#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::stringChunk(FILE * Stream)//movimientos de la pagina
{
    Main_Data::data_words = new RPG::Glossary();
    do {
        ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
        if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
        switch (ChunkInfo.ID) { // tipo de la primera dimencion
        case  Enemy_encounter://0x01,
            Main_Data::data_words->enemy_encounter = ReadString(Stream, ChunkInfo.Length);
            break;
        case Headstart_attack://0x02,
            Main_Data::data_words->headstart_attack = ReadString(Stream, ChunkInfo.Length);
            break;
        case Escape_success://0x03,
            Main_Data::data_words->escape_success = ReadString(Stream, ChunkInfo.Length);
            break;
        case Escape_failure://0x04,
            Main_Data::data_words->escape_failure = ReadString(Stream, ChunkInfo.Length);
            break;
        case Battle_victory://0x05,
            Main_Data::data_words->battle_victory = ReadString(Stream, ChunkInfo.Length);
            break;
        case Battle_defeat://0x06,
            Main_Data::data_words->battle_defeat = ReadString(Stream, ChunkInfo.Length);
            break;
        case Experience_received://0x07,
            Main_Data::data_words->experience_received = ReadString(Stream, ChunkInfo.Length);
            break;
        case Money_recieved_A://0x08,
            Main_Data::data_words->gold_received_A = ReadString(Stream, ChunkInfo.Length);
            break;
        case Money_recieved_B://0x09,
            Main_Data::data_words->gold_received_B = ReadString(Stream, ChunkInfo.Length);
            break;
        case Item_recieved://0x0A,
            Main_Data::data_words->item_received = ReadString(Stream, ChunkInfo.Length);
            break;
        case Attack_message://0x0B,
            Main_Data::data_words->attack_message = ReadString(Stream, ChunkInfo.Length);
            break;
        case Ally_critical_hit://0x0C,
            Main_Data::data_words->ally_crit_hit = ReadString(Stream, ChunkInfo.Length);
            break;
        case Enemy_critical_hit://0x0D,
            Main_Data::data_words->enemy_crit_hit = ReadString(Stream, ChunkInfo.Length);
            break;
        case Defend_message://0x0E,
            Main_Data::data_words->defend = ReadString(Stream, ChunkInfo.Length);
            break;
        case Watch_message://0x0F,
            Main_Data::data_words->watch = ReadString(Stream, ChunkInfo.Length);
            break;
        case Gathering_energy://0x10,
            Main_Data::data_words->gathering_energy = ReadString(Stream, ChunkInfo.Length);
            break;
        case Sacrificial_attack://0x11,
            Main_Data::data_words->sacrificial_attack = ReadString(Stream, ChunkInfo.Length);
            break;
        case Enemy_escape://0x12,
            Main_Data::data_words->enemy_escape = ReadString(Stream, ChunkInfo.Length);
            break;
        case Enemy_transform://0x13,
            Main_Data::data_words->enemy_transform = ReadString(Stream, ChunkInfo.Length);
            break;
        case Enemy_damaged://0x14,
            Main_Data::data_words->enemy_damaged = ReadString(Stream, ChunkInfo.Length);
            break;
        case Enemy_undamaged://0x15,
            Main_Data::data_words->enemy_undamaged = ReadString(Stream, ChunkInfo.Length);
            break;
        case Ally_damaged://0x16,
            Main_Data::data_words->ally_damaged = ReadString(Stream, ChunkInfo.Length);
            break;
        case Ally_undamaged://0x17,
            Main_Data::data_words->ally_undamaged = ReadString(Stream, ChunkInfo.Length);
            break;
        case Skill_failure_A://0x18,
            Main_Data::data_words->skill_failure_A = ReadString(Stream, ChunkInfo.Length);
            break;
        case Skill_failure_B://0x19,
            Main_Data::data_words->skill_failure_B = ReadString(Stream, ChunkInfo.Length);
            break;
        case Skill_failure_C://0x1A,
            Main_Data::data_words->skill_failure_C = ReadString(Stream, ChunkInfo.Length);
            break;
        case Attack_dodged://0x1B,
            Main_Data::data_words->attack_dodged = ReadString(Stream, ChunkInfo.Length);
            break;
        case Item_use://0x1C,
            Main_Data::data_words->item_use = ReadString(Stream, ChunkInfo.Length);
            break;
        case Stat_recovery://0x1D,
            Main_Data::data_words->stat_recovery = ReadString(Stream, ChunkInfo.Length);
            break;
        case Stat_increase://0x1E,
            Main_Data::data_words->stat_increase = ReadString(Stream, ChunkInfo.Length);
            break;
        case Stat_decrease://0x1F,
            Main_Data::data_words->stat_decrease = ReadString(Stream, ChunkInfo.Length);
            break;
        case Ally_lost_via_absorb://0x20,
            Main_Data::data_words->ally_lost_via_absorb = ReadString(Stream, ChunkInfo.Length);
            break;
        case Enemy_lost_via_absorb://0x21,
            Main_Data::data_words->enemy_lost_via_absorb = ReadString(Stream, ChunkInfo.Length);
            break;
        case Resistance_increase://0x22,
            Main_Data::data_words->resistance_increase = ReadString(Stream, ChunkInfo.Length);
            break;
        case Resistance_decrease://0x23,
            Main_Data::data_words->resistance_decrease = ReadString(Stream, ChunkInfo.Length);
            break;
        case Level_up_message://0x24,
            Main_Data::data_words->level_up = ReadString(Stream, ChunkInfo.Length);
            break;
        case Skill_learned://0x25,
            Main_Data::data_words->skill_learned = ReadString(Stream, ChunkInfo.Length);
            break;
        case Shop_greeting ://0x29,
            Main_Data::data_words->shop_greeting = ReadString(Stream, ChunkInfo.Length);
            break;
        case Shop_regreeting ://0x2A,
            Main_Data::data_words->shop_regreeting = ReadString(Stream, ChunkInfo.Length);
            break;
        case Buy_message ://0x2B,
            Main_Data::data_words->buy = ReadString(Stream, ChunkInfo.Length);
            break;
        case Sell_message ://0x2C,
            Main_Data::data_words->sell = ReadString(Stream, ChunkInfo.Length);
            break;
        case Leave_message ://0x2D,
            Main_Data::data_words->leave = ReadString(Stream, ChunkInfo.Length);
            break;
        case Buying_message ://0x2E,
            Main_Data::data_words->buying = ReadString(Stream, ChunkInfo.Length);
            break;
        case Quantity_to_buy ://0x2F,
            Main_Data::data_words->quantity_to_buy = ReadString(Stream, ChunkInfo.Length);
            break;
        case Purchase_end ://0x30,
            Main_Data::data_words->purchase_end = ReadString(Stream, ChunkInfo.Length);
            break;
        case Selling_message ://0x31,
            Main_Data::data_words->selling = ReadString(Stream, ChunkInfo.Length);
            break;
        case Quantity_to_sell ://0x32,
            Main_Data::data_words->quantity_to_sell = ReadString(Stream, ChunkInfo.Length);
            break;
        case Selling_end ://0x33,
            Main_Data::data_words->selling_end = ReadString(Stream, ChunkInfo.Length);
            break;
        case Shop_greeting_2 ://0x36,
            Main_Data::data_words->shop_greeting_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Shop_regreeting_2 ://0x37,
            Main_Data::data_words->shop_regreeting_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Buy_message_2 ://0x38,
            Main_Data::data_words->buy_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Sell_message_2 ://0x39,
            Main_Data::data_words->sell_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Leave_message_2 ://0x3A,
            Main_Data::data_words->leave_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Buying_message_2 ://0x3B,
            Main_Data::data_words->buying_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Quantity_to_buy_2 ://0x3C,
            Main_Data::data_words->quantity_to_buy_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Purchase_end_2 ://0x3D,
            Main_Data::data_words->purchase_end_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Selling_message_2 ://0x3E,
            Main_Data::data_words->selling_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Quantity_to_sell_2 ://0x3F,
            Main_Data::data_words->quantity_to_sell_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Selling_end_2 ://0x40,
            Main_Data::data_words->selling_end_2 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Shop_greeting_3 ://0x43,
            Main_Data::data_words->shop_greeting_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Shop_regreeting_3 ://0x44,
            Main_Data::data_words->shop_regreeting_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Buy_message_3 ://0x45,
            Main_Data::data_words->buy_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Sell_message_3 ://0x46,
            Main_Data::data_words->sell_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Leave_message_3 ://0x47,
            Main_Data::data_words->leave_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Buying_message_3 ://0x48,
            Main_Data::data_words->buying_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Quantity_to_buy_3 ://0x49,
            Main_Data::data_words->quantity_to_buy_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Purchase_end_3 ://0x4A,
            Main_Data::data_words->purchase_end_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Selling_message_3 ://0x4B,
            Main_Data::data_words->selling_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Quantity_to_sell_3 ://0x4C,
            Main_Data::data_words->quantity_to_sell_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Selling_end_3 ://0x4D,
            Main_Data::data_words->selling_end_3 = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_A_Greeting_A://0x50,
            Main_Data::data_words->inn_A_greeting_A = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_A_Greeting_B://0x51,
            Main_Data::data_words->inn_A_greeting_B = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_A_Greeting_C://0x52,
            Main_Data::data_words->inn_A_greeting_C = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_A_Accept://0x53,
            Main_Data::data_words->inn_A_accept = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_A_Cancel://0x54,
            Main_Data::data_words->inn_A_cancel = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_B_Greeting_A://0x55,
            Main_Data::data_words->inn_B_greeting_A = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_B_Greeting_B://0x56,
            Main_Data::data_words->inn_B_greeting_B = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_B_Greeting_C://0x57,
            Main_Data::data_words->inn_B_greeting_C = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_B_Accept://0x58,
            Main_Data::data_words->inn_B_accept = ReadString(Stream, ChunkInfo.Length);
            break;
        case Inn_B_Cancel://0x59,
            Main_Data::data_words->inn_B_cancel = ReadString(Stream, ChunkInfo.Length);
            break;
        case Loose_items://0x5C,
            Main_Data::data_words->loose_items = ReadString(Stream, ChunkInfo.Length);
            break;
        case Equipped_items://0x5D,
            Main_Data::data_words->equipped_items = ReadString(Stream, ChunkInfo.Length);
            break;
        case Monetary_Unit://0x5F,
            Main_Data::data_words->gold_unit = ReadString(Stream, ChunkInfo.Length);
            break;
        case Combat_Command://0x65,
            Main_Data::data_words->combat_command = ReadString(Stream, ChunkInfo.Length);
            break;
        case Combat_Auto://0x66,
            Main_Data::data_words->combat_auto = ReadString(Stream, ChunkInfo.Length);
            break;
        case Combat_Run://0x67,
            Main_Data::data_words->combat_run = ReadString(Stream, ChunkInfo.Length);
            break;
        case Command_Attack://0x68,
            Main_Data::data_words->command_attack = ReadString(Stream, ChunkInfo.Length);
            break;
        case Command_Defend://0x69,
            Main_Data::data_words->command_defend = ReadString(Stream, ChunkInfo.Length);
            break;
        case Command_Item://0x6A,
            Main_Data::data_words->command_item = ReadString(Stream, ChunkInfo.Length);
            break;
        case Command_Skill://0x6B,
            Main_Data::data_words->command_skill = ReadString(Stream, ChunkInfo.Length);
            break;
        case Menu_Equipment://0x6C,
            Main_Data::data_words->menu_equipment = ReadString(Stream, ChunkInfo.Length);
            break;
        case Menu_Save://0x6E,
            Main_Data::data_words->menu_save = ReadString(Stream, ChunkInfo.Length);
            break;
        case Menu_Quit://0x70,
            Main_Data::data_words->menu_quit = ReadString(Stream, ChunkInfo.Length);

            break;
        case New_Game://0x72,
            Main_Data::data_words->new_game = ReadString(Stream, ChunkInfo.Length);
            break;
        case Load_Game://0x73,
            Main_Data::data_words->load_game = ReadString(Stream, ChunkInfo.Length);
            break;
        case Exit_to_Windows://0x75,
            Main_Data::data_words->exit_game = ReadString(Stream, ChunkInfo.Length);
            break;
        case Level://0x7B,
            Main_Data::data_words->level = ReadString(Stream, ChunkInfo.Length);
            break;
        case Health://0x7C,
            Main_Data::data_words->health = ReadString(Stream, ChunkInfo.Length);
            break;
        case Mana://0x7D,
            Main_Data::data_words->mana = ReadString(Stream, ChunkInfo.Length);
            break;
        case Normal_status://0x7E,
            Main_Data::data_words->normal_status = ReadString(Stream, ChunkInfo.Length);
            break;
        case Experience ://0x7F,//(short)
            Main_Data::data_words->experience = ReadString(Stream, ChunkInfo.Length);
            break;
        case Level_short ://0x80,//(short)
            Main_Data::data_words->level_short = ReadString(Stream, ChunkInfo.Length);
            break;
        case Health_short ://0x81,//(short)
            Main_Data::data_words->health_short = ReadString(Stream, ChunkInfo.Length);
            break;
        case Mana_short ://0x82,//(short)
            Main_Data::data_words->mana_short = ReadString(Stream, ChunkInfo.Length);
            break;
        case Mana_cost://0x83,
            Main_Data::data_words->mana_cost = ReadString(Stream, ChunkInfo.Length);
            break;
        case Attack://0x84,
            Main_Data::data_words->atk = ReadString(Stream, ChunkInfo.Length);
            break;
        case Defense://0x85,
            Main_Data::data_words->pdef = ReadString(Stream, ChunkInfo.Length);
            break;
        case Mind://0x86,
            Main_Data::data_words->mind = ReadString(Stream, ChunkInfo.Length);
            break;
        case Agility://0x87,
            Main_Data::data_words->agi = ReadString(Stream, ChunkInfo.Length);
            break;
        case Weapon://0x88,
            Main_Data::data_words->weapon = ReadString(Stream, ChunkInfo.Length);
            break;
        case Shield://0x89,
            Main_Data::data_words->shield = ReadString(Stream, ChunkInfo.Length);
            break;
        case Armor://0x8A,
            Main_Data::data_words->armor = ReadString(Stream, ChunkInfo.Length);
            break;
        case Helmet://0x8B,
            Main_Data::data_words->helmet = ReadString(Stream, ChunkInfo.Length);
            break;
        case Accessory://0x8C,
            Main_Data::data_words->accessory = ReadString(Stream, ChunkInfo.Length);
            break;
        case Save_game_message://0x92,
            Main_Data::data_words->save_game_conf = ReadString(Stream, ChunkInfo.Length);
            break;
        case Load_game_message://0x93,
            Main_Data::data_words->load_game_conf = ReadString(Stream, ChunkInfo.Length);
            break;
        case Exit_game_message://0x94,
            Main_Data::data_words->exit_game_conf = ReadString(Stream, ChunkInfo.Length);
            break;
        case File_name://0x97,
            Main_Data::data_words->file_name = ReadString(Stream, ChunkInfo.Length);
            break;
        case General_Yes://0x98,
            Main_Data::data_words->general_yes = ReadString(Stream, ChunkInfo.Length);
            break;
        case General_No://0x99
            Main_Data::data_words->general_no = ReadString(Stream, ChunkInfo.Length);
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
