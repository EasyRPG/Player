#include "ldb_reader.h"

bool LDB_reader::Load(std::string Filename, LDB_data * data)
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
    //GetNextChunk(Stream,data);
    fclose(Stream);
    return true;
}

void LDB_reader::heroskillChunk(FILE * Stream, Actor* hero)
{
    int id,datatoread=0,datareaded=0;
    skill_block skill;
   // static std::vector <Magicblock> vecSkills;
    datatoread = ReadCompressedInteger(Stream); //numero de datos
    while (datatoread>datareaded) { //si no hay mas en el array
        id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        skill.skill_id=1; //default

        do {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case MagicblockChunk_Level:
                skill.level = ReadCompressedInteger(Stream);
                break;
            case MagicblockChunk_Spell_ID:
                skill.skill_id = ReadCompressedInteger(Stream);
                break;
            case CHUNK_LDB_END_OF_BLOCK:
                break;
            default:
                bool return_value;
                while (ChunkInfo.Length--)
                {
                    return_value = fread(&Void, 1, 1, Stream);
                }
                break;
            }
        } while (ChunkInfo.ID!=0);
        hero->skills.push_back(skill);
        datareaded++;
    }
    ChunkInfo.ID=1;
}

void LDB_reader::heroChunk(FILE * Stream)
{
    int id,datatoread=0,datareaded=0,comands=0;
    int id_number = 0;
    int levels;
    short dat;
    //std::vector<stcHero*> *vecHero;
    Actor *hero_data;
    
    //vecHero = new std::vector<stcHero*>();
    datatoread = ReadCompressedInteger(Stream);//numero de datos
    while (datatoread > datareaded) // si no hay mas en el array
    { 
        hero_data = new Actor();
        id_number++;
        //hero_data->set_defaults();
        id = ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {
            ChunkInfo.ID = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            //printf("%x\n", ChunkInfo.ID);
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case CHUNK_Name:
                hero_data->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case CHUNK_Class:
                hero_data->title_name = ReadString(Stream, ChunkInfo.Length);
                break;
            case CHUNK_Graphicfile:
                hero_data->character_name = ReadString(Stream, ChunkInfo.Length);

                break;
            case CHUNK_Graphicindex:
                hero_data->graphic_index = (ReadCompressedInteger(Stream);
                break;
            case CHUNK_Transparent:
                hero_data->transparent = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_Startlevel:
                hero_data->initial_level = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Maxlevel:
                hero_data->final_level = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Crithit:
                hero_data->critical_hit = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Hitchance:
                hero_data->critical_chance = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Facegraphic:
                hero_data->face_name = ReadString(Stream, ChunkInfo.Length);
                break;
            case CHUNK_Faceindex:
                hero_data->face_index = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Dualwield:
                hero_data->double_weapon = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_Fixedequipment:
                hero_data->lock_equipment = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_AI:
                hero_data->ia_control = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_Highdefense:
                hero_data->high_defense = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_Statisticscurves:
                bool return_value;
                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->stats[0].push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->stats[1].push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->stats[2].push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->stats[3].push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->stats[4].push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->stats[5].push_back(dat);
                    levels-=2;
                }
                break;
            case CHUNK_EXPBaseline:
                hero_data->exp_base_line = ReadCompressedInteger(Stream);
                break;
            case CHUNK_EXPAdditional:
                hero_data->exp_additional = ReadCompressedInteger(Stream);
                break;
            case CHUNK_EXPCorrection:
                hero_data->exp_correction = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Startequip:
                bool return_value;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->weapon_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->armor1_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->armor3_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->armor2_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->armor4_id = dat;
                break;
            case CHUNK_Skills:
                heroskillChunk(Stream, hero_data);
                break;
            case CHUNK_RenameMagic:
                hero_data->rename_skill = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Magicname:
                hero_data->skill_name = ReadString(Stream, ChunkInfo.Length);
                break;
            /* TODO: Save those last blocks */
            case CHUNK_Condlength:
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Condeffects:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    hero_data->vc_ch_Condeffects.push_back(Void);
                }
                break;
            case CHUNK_Attriblength:
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Attribeffects:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    hero_data->attribute_effects.push_back(Void);
                }
                break;
            case CHUNK_Combat_Command://0x50
                bool return_value;
                while (ChunkInfo.Length--) { //4 chars
                    return_value = fread(&comands, 4, 1, Stream);
                    hero_data->combat_command.push_back(comands);
                    ChunkInfo.Length-=3;
                }
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
        //vecHero->push_back(hero_data);
        Main_Data::data_actors.push_back(hero_data);
        datareaded++;
    }
    return;
}