#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::mosteractionChunk(FILE * Stream, RPG::Enemy* e)
{
    int datatoread=0,datareaded=0;
    E_Action *action;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        action = new E_Action();
        if (action == NULL)
        {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }
        action->id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case	MonsterActionsChunk_Action:
                action->kind = ReadCompressedInteger(Stream);
                break;
            case MagicblockChunk_Spell_ID:
                action->basic = ReadCompressedInteger(Stream);
                break;
            case MonsterActionsChunk_Skill_ID:
                action->skill_id = ReadCompressedInteger(Stream);
                break;
            case MonsterActionsChunk_Monster_ID:
                action->monster_id = ReadCompressedInteger(Stream);
                break;
            case MonsterActionsChunk_Condition:
                action->condition = ReadCompressedInteger(Stream);
                break;
            case MonsterActionsChunk_Lower_limit:
                action->lower_limit = ReadCompressedInteger(Stream);
                break;
            case MonsterActionsChunk_Upper_limit:
                action->upper_limit = ReadCompressedInteger(Stream);
                break;
            case MonsterActionsChunk_Priority:
                action->priority = ReadCompressedInteger(Stream);
                break;
            case CHUNK_LDB_END_OF_BLOCK:
                break;
            default:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);   
                }
                break;
            }
        } while (ChunkInfo.ID!=0);
        e->actions.push_back(action);
        datareaded++;
    }
    ChunkInfo.ID	 =1;
}

void LDB_reader::mosterChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;    

    RPG::Enemy *enemy;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    //printf("Numero de datos -> %d\n", datatoread);
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        enemy = new RPG::Enemy();
        if (enemy == NULL)
        {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }
        enemy->id = ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {            
            ChunkInfo.ID = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            //printf("%x\n", ChunkInfo.ID);
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case MonsterChunk_Name:
                enemy->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case MonsterChunk_Graphicfile:
                enemy->battler_name = ReadString(Stream, ChunkInfo.Length);
                break;
            case MonsterChunk_Huealteration://0x03,
                enemy->battler_hue = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_MaxHP://0x04,
                enemy->maxhp = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_MaxMP://0x05,
                enemy->maxmp = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Attack://0x06,
                enemy->str = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Defense://0x07,
                enemy->pdef = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Mind://0x08,
                enemy->iint = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Speed://0x09,
                enemy->agi = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Translucentgraphic://0x0A,
                enemy->transparent = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Experience://0x0B,
                enemy->exp = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Gold://0x0C,
                enemy->gold = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_SpoilsitemID://0x0D,
                enemy->item_id = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Spoilschance://0x0E,
                enemy->treasure_prob = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Canusecriticalhits://0x15,
                enemy->critical_hit = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Criticalhitchance://0x16,
                enemy->critical_hit_chance = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Usuallymiss://0x1A,
                enemy->miss = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Airborne://0x1C,
                enemy->levitate = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Conditionslength://0x1F,
                trash = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Conditionseffects://0x20,
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    enemy->state_ranks.push_back(Void);
                }
                break;
            case MonsterChunk_Attributeslength://0x21,
                trash = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Attributeseffect://0x22,
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    enemy->attribute_ranks.push_back(Void);
                }
                break;
            case MonsterChunk_Actionslist://0x2A
                mosteractionChunk(Stream, enemy);
                break;
            case CHUNK_LDB_END_OF_BLOCK:
                break;
            default:
                // saltate un pedazo del tamaño de la longitud
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);   
                }
                break;
            }
        } while (ChunkInfo.ID!=0);
        datareaded++;
        Main_Data::data_enemies.push_back(enemy);
    }
}
