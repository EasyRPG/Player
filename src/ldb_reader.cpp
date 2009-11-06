#include "ldb_reader.h"
#include "rpg_sound.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

bool LDB_reader::load(const std::string& Filename) 
{
    // Open map file to read
    FILE * Stream;// apertura de archivo
    Stream = fopen(Filename.c_str(), "rb");
    if (Stream == NULL) {
        std::cerr << "Couldn't find LDB database. Aborting" << std::endl;
        exit(1);
    }
    std::string Header = ReadString(Stream); // lectura de cabezera
    if (Header != "LcfDataBase") // comparacion con cabezera del mapa
    { // si no concuerda imprime un error y finaliza
        printf("Reading error: File is not a valid RPG2000 database\n");
        fclose(Stream);
        return false;
    }
    GetNextChunk(Stream);
    fclose(Stream);
    return true;
}

void LDB_reader::GetNextChunk(FILE * Stream)
{
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    // Loop while we haven't reached the end of the file
    while (!feof(Stream)) {
        ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
        ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
        if (ChunkInfo.Length>0)
            switch (ChunkInfo.ID) { // segun el tipo

            case CHUNK_Hero_data:
                heroChunk(Stream);
                break;
            case CHUNK_Skill:
                skillChunk(Stream);
                break;
            case CHUNK_Item_data:
                itemChunk(Stream);
                break;
            case CHUNK_Monster:
                mosterChunk(Stream);
                break;
            case CHUNK_MonsterP:
                mosterpartyChunk(Stream);
                break;
            case CHUNK_Terrain:
                terrainChunk(Stream);
                break;
            case CHUNK_Attribute:
                attributeChunk(Stream);
                break;
            case CHUNK_States:
                statesChunk(Stream);
                break;
            case CHUNK_System:
                data->System_dat = systemChunk(Stream);
                break;
            /*case CHUNK_Animation:
                data->animations = animationChunk(Stream);
                break;
            case CHUNK_Tileset:
                data->tilesets = tilesetChunk(Stream);
                break;
            case CHUNK_String:
                data->Glosary = stringChunk(Stream);
                break;
            case CHUNK_Event:
                data->Event = EventChunk(Stream);
                break;
            case CHUNK_Switch:
                data->Switch_Names = Switch_VariableChunk(Stream);
                break;
            case CHUNK_Variable:
                data->Variable_Names = Switch_VariableChunk(Stream);
                break;
            case CHUNK_Event1://no existe
                while (ChunkInfo.Length--) {
                    bool return_value;
                    return_value = fread(&Void, 1, 1, Stream);
                    printf("%d",Void);
                }

                break;
            case CHUNK_Event2://no existe
                while (ChunkInfo.Length--) {
                    bool return_value;
                    return_value = fread(&Void, 1, 1, Stream);
                    printf("%d",Void);
                }
                break;
            case CHUNK_Event3://no existe
                while (ChunkInfo.Length--) {
                    bool return_value;
                    return_value = fread(&Void, 1, 1, Stream);
                    printf("%d",Void);
                }
                break;
            case CHUNK_Comand:
                data->Combatcommands=Comand_Chunk(Stream);
                break;
            case CHUNK_Profession:
                data->Professions=Profession_Chunk(Stream);
                break;
            case CHUNK_Profession2://no existe

                while (ChunkInfo.Length--) {
                    bool return_value;
                    return_value = fread(&Void, 1, 1, Stream);
                    printf("%d ",Void);
                }
                break;
            case CHUNK_Fightanim:
                data->Fightanims=Fightanim_Chunk(Stream);
                break;
            case CHUNK_LDB_END_OF_BLOCK:
                break;*/
            default:
                // saltate un pedazo del tamaño de la longitud
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                }
                break;
            }

    }
}
