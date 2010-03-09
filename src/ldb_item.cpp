#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::itemChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    RPG::Item *item;
    datatoread=ReadCompressedInteger(Stream);
    while (datatoread>datareaded) 
    {
        item = new RPG::Item();
        item->id = ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {            
            ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case ItemChunk_Name:
                item->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case ItemChunk_Description:
                item->description = ReadString(Stream, ChunkInfo.Length);
                break;
            case ItemChunk_Type:
                item->type = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Cost:
                item->price = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Uses:
                item->uses = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attack:
                item->atk_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Defense:
                item->pdef_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Mind:
                item->int_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Speed:
                item->agi_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Equip:
                item->two_handed = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPcost:
                item->sp_cost = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Chancetohit:
                item->hit = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Criticalhit:
                item->critical_hit = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Battleanimation:
                item->animation_id = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Preemptiveattack:
                item->preemptive = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Doubleattack:
                item->dual_attack = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attackallenemies:
                item->attack_all = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Ignoreevasion:
                item->ignore_evasion = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Preventcriticalhits:
                item->prevent_critical = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Raiseevasion:
                item->raise_evasion = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPusecutinhalf:
                item->half_sp_cost = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Noterraindamage:
                item->no_terrain_damage = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Healsparty:
                item->entire_party = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_HPrecovery ://0x20,
                item->recover_hp_rate = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_HPrecoveryvalue://0x21,
                item->recover_hp = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPrecovery://0x22,
                item->recover_sp_rate = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPrecoveryvalue://0x23,
                item->recover_sp = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Useonfieldonly://0x25,
                item->ocassion_field = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Onlyondeadheros://0x26,
                item->ko_only = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MaxHPmodify://0x29,
                item->max_hp_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MaxMPmodify://0x2A,
                item->max_sp_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attackmodify://0x2B,
                item->atk_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Defensemodify://0x2C,
                item->pdef_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Mindmodify://0x2D,
                item->int_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Speedmodify://0x2E,
                item->agi_points = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Usagemessage://0x33,
                item->using_messsage = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Switchtoturnon://0x37,
                item->switch_id = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Useonfield://0x39,
                item->ocassion_field = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Useinbattle://0x3A,
                item->ocassion_battle = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Heroeslength://0x3D,
                trash = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Heroescanuse:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    item->actor_set.push_back(Void);
                }
                break;
            case ItemChunk_Conditionslength://0x3F,
                trash = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Conditionchanges:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    item->state_set.push_back(Void);
                }
                break;
            case ItemChunk_Attributeslength://0x41,
                trash = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attributes:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    item->attribute_set.push_back(Void);
                }
                break;
            case ItemChunk_Chancetochange://0x43
                item->state_chance = ReadCompressedInteger(Stream);
                break;
            case CHUNK_LDB_END_OF_BLOCK:
                break;
            default:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                }
                break;
            }
        } while (ChunkInfo.ID!=0);
        datareaded++;
        Main_Data::data_objects.push_back(item);
    }
}
