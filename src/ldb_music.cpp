#include "ldb_reader.h"
#include "rpg_music.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::musicChunk(FILE * Stream, RPG::Music*& music)
{
    music = new RPG::Music();
    do {
        ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
        if (ChunkInfo.ID!=0)
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

        switch (ChunkInfo.ID) { // segun el tipo
        case 0x01:
            music->name     = ReadString(Stream, ChunkInfo.Length);
            break;
        case 0x03:
            music->volume     = ReadCompressedInteger(Stream);
            break;
        case 0x04:
            music->tempo     = ReadCompressedInteger(Stream);
            break;
        case 0x05:
            music->balance     = ReadCompressedInteger(Stream);
            break;
        case 0x00:
            break;
        default:
            while (ChunkInfo.Length--) {
                return_value = fread(&Void, 1, 1, Stream);
            }
            break;
        }
    } while (ChunkInfo.ID!=0);
    ChunkInfo.ID=1;
}
