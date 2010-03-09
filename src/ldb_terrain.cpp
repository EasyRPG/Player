#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::terrainChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    std::string name;
    RPG::Terrain *terrain;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        terrain = new RPG::Terrain();  
        terrain->id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {
            ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case TerrainChunk_Name:
                terrain->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case TerrainChunk_Damageontravel://0x02,
                terrain->damage = ReadCompressedInteger(Stream);
                break;
            case TerrainChunk_Encounterate://0x03,
                terrain->encounter_rate = ReadCompressedInteger(Stream);
                break;
            case TerrainChunk_Battlebackground://0x04,
                terrain->battle_background = ReadString(Stream, ChunkInfo.Length);
                break;
            case TerrainChunk_Skiffmaypass://0x05,
                terrain->ship_pass = ReadCompressedInteger(Stream);
                break;
            case TerrainChunk_Boatmaypass://0x06,
                terrain->boat_pass = ReadCompressedInteger(Stream);
                break;
            case TerrainChunk_Airshipmaypass://0x07,
                terrain->airship_pass = ReadCompressedInteger(Stream);
                break;
            case TerrainChunk_Airshipmayland://0x09,
                terrain->airship_land = ReadCompressedInteger(Stream);
                break;
            case TerrainChunk_Heroopacity://0x0B
                terrain->chara_opacity = ReadCompressedInteger(Stream);
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
        Main_Data::data_terrains.push_back(terrain);
    }
}
