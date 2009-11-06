#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::attributeChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;

    RPG::Attribute *attribute;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        attribute = new RPG::Attribute();
        if (attribute == NULL) {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }
        
        attribute->id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {
            ChunkInfo.ID = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case AttributeChunk_Name:
                attribute->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case AttributeChunk_Type://0x02,
                attribute->type = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_A_damage://0x0B,
                attribute->a_rate = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_B_damage://0x0C,
                attribute->b_rate = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_C_damage://0x0F,
                attribute->c_rate = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_D_damage://0x0F,
                attribute->d_rate = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_E_damage://0x0F
                attribute->e_rate = ReadCompressedInteger(Stream);
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
        Main_Data::data_attributes.push_back(attribute);
    }
}
