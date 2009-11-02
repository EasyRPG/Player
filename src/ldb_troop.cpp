#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::MonsterPartyMonsterChunk(FILE * Stream, RPG::Troop *trp)
{
    int id,datatoread=0,datareaded=0;
    RPG::Member *monster = new RPG::Member();
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) { // si no hay mas en el array
        id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case	Monster_ID:
                monster->enemy_id = ReadCompressedInteger(Stream);
                break;
            case X_position:
                monster->x = ReadCompressedInteger(Stream);
                break;
            case Y_position:
                monster->y = ReadCompressedInteger(Stream);
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
        trp->members.push_back(*monster);
        delete monster;
        datareaded++;
    }
    ChunkInfo.ID	 =1;
}

void LDB_reader::MonsterPartyEventconditionChunk(FILE * Stream, RPG::Page* pgs)
{
    short dat;
    do {
        ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
        if (ChunkInfo.ID!=0)
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

        switch (ChunkInfo.ID) { // segun el tipo
        case Condition_flags:// en el 2003 se usan 2 chars
            if (ChunkInfo.Length==2)
                pgs->condition.flags = fread(&dat, 2, 1, Stream);
            else
                pgs->condition.flags = ReadCompressedInteger(Stream);
            break;
        case Switch_A:
            pgs->condition.switch_a	 = ReadCompressedInteger(Stream);
            break;
        case Turn_number_A:
            pgs->condition.turn_number_a	 = ReadCompressedInteger(Stream);
            break;
        case Lower_limit:
            pgs->condition.lower_limit	 = ReadCompressedInteger(Stream);
            break;
        case Upper_limit:
            pgs->condition.upper_limit	 = ReadCompressedInteger(Stream);
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
    ChunkInfo.ID=1;
}

void LDB_reader::MonsterPartyevent_pageChunk(FILE * Stream, RPG::Troop *trp)
{
    int id,datatoread=0,datareaded=0;
    RPG::Page Monsterevent;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) { // si no hay mas en el array
        id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case Page_conditions:
                MonsterPartyEventconditionChunk(Stream, &Monsterevent);
                break;
            case Event_length:
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case Event_Monster:
             /* TODO: Events */
                //Monsterevent.vcEvent_comand =  Event_parser.EventcommandChunk(Stream);
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
        trp->pages.push_back(Monsterevent);
        datareaded++;
    }
    ChunkInfo.ID	 =1;
}

void LDB_reader::mosterpartyChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;    
    RPG::Troop *enemy_group;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        enemy_group = new RPG::Troop();
        if (enemy_group == NULL)
        {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }
        enemy_group->id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream);
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream);
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case MonsterPartyChunk_Name:
                enemy_group->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case MonsterPartyChunk_Monsterdata://0x02,
                MonsterPartyMonsterChunk(Stream, enemy_group);
                break;
            case MonsterPartyChunk_Terrainlength://0x04,
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case MonsterPartyChunk_Terraindata://0x05,
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    enemy_group->terrain_data.push_back(Void);
                }
                break;
            case MonsterPartyChunk_eventpages://0x0B
                MonsterPartyevent_pageChunk(Stream, enemy_group);
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
        datareaded++;
        Main_Data::data_troops.push_back(enemy_group);
    }
}
