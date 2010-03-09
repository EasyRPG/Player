#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::heroskillChunk(FILE * Stream, RPG::Actor*& hero)
{
    int id,datatoread=0,datareaded=0;
    RPG::Learning skill;
    datatoread = ReadCompressedInteger(Stream); //numero de datos
    while (datatoread>datareaded) { //si no hay mas en el array
        id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        skill.skill_id=1; //default

        do {
            ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
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
    int datatoread=0,datareaded=0;
    int levels;
    short dat;
    RPG::Actor *hero_data;
    
    datatoread = ReadCompressedInteger(Stream);//numero de datos
    while (datatoread > datareaded) // si no hay mas en el array
    { 
        hero_data = new RPG::Actor();
        hero_data->id = ReadCompressedInteger(Stream);//lectura de id 1 de array
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
                hero_data->title = ReadString(Stream, ChunkInfo.Length);
                break;
            case CHUNK_Graphicfile:
                hero_data->character_name = ReadString(Stream, ChunkInfo.Length);

                break;
            case CHUNK_Graphicindex:
                hero_data->character_index = ReadCompressedInteger(Stream);
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
                hero_data->critical_hit_chance = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Facegraphic:
                hero_data->face_name = ReadString(Stream, ChunkInfo.Length);
                break;
            case CHUNK_Faceindex:
                hero_data->face_index = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Dualwield:
                hero_data->two_swords_style = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_Fixedequipment:
                hero_data->fix_equipment = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_AI:
                hero_data->auto_battle = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_Highdefense:
                hero_data->super_guard = (ReadCompressedInteger(Stream)) ? true : false;
                break;
            case CHUNK_Statisticscurves:
                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->parameter_maxhp.push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->parameter_maxsp.push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->parameter_attack.push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->parameter_defense.push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->parameter_spirit.push_back(dat);
                    levels-=2;
                }

                levels=ChunkInfo.Length/6;
                while (levels > 0) {
                    return_value = fread(&dat, 2, 1, Stream);
                    hero_data->parameter_agility.push_back(dat);
                    levels-=2;
                }
                break;
            case CHUNK_EXPBaseline:
                hero_data->exp_base = ReadCompressedInteger(Stream);
                break;
            case CHUNK_EXPAdditional:
                hero_data->exp_inflation = ReadCompressedInteger(Stream);
                break;
            case CHUNK_EXPCorrection:
                hero_data->exp_correction = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Startequip:
                
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->weapon_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->shield_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->armor_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->head_id = dat;
                return_value = fread(&dat, 2, 1, Stream);
                hero_data->accessory_id = dat;
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
                
                trash = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Condeffects:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    hero_data->condition_effects.push_back(Void);
                }
                break;
            case CHUNK_Attriblength:
                trash = ReadCompressedInteger(Stream);
                break;
            case CHUNK_Attribeffects:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    hero_data->attribute_effects.push_back(Void);
                }
                break;
                
            // What is this?
            /*case CHUNK_Combat_Command://0x50
                while (ChunkInfo.Length--) { //4 chars
                    return_value = fread(&comands, 4, 1, Stream);
                    hero_data->combat_command.push_back(comands);
                    ChunkInfo.Length-=3;
                }
                break;*/
            case CHUNK_LDB_END_OF_BLOCK:
                break;
            default:
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
