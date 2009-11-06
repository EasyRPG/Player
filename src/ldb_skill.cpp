#include "ldb_reader.h"

namespace {
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    bool return_value;
    int trash;
}

void LDB_reader::skillChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    RPG::Skill *skill;
    datatoread=ReadCompressedInteger(Stream);//numero de datos

    while (datatoread>datareaded) 
    { // si no hay mas en el array
        skill = new RPG::Skill();
        if (skill == NULL) {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }
        skill->id = ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {
            ChunkInfo.ID = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case SkillChunk_Name:
                skill->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case SkillChunk_Description:
                skill->description = ReadString(Stream, ChunkInfo.Length);
                break;
            case SkillChunk_text:
                skill->using_message1 = ReadString(Stream, ChunkInfo.Length);
                break;
            case SkillChunk_textline:
                skill->using_message2 = ReadString(Stream, ChunkInfo.Length);
                break;
            case SkillChunk_Failure:
                skill->failure_message = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Spelltype:
                skill->type = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Cost:
                skill->sp_cost = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Range:
                skill->scope = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Switch:
                skill->switch_id = ReadCompressedInteger(Stream);
                break;
			// What is this?
            /*case SkillChunk_Battleanimation:
                skill->animation1_id = ReadCompressedInteger(Stream);
                break;*/
            case SkillChunk_Soundeffect:
                soundChunk(Stream, skill->sound_effect);
                break;
            case SkillChunk_Fieldusage:
                skill->occasion_field = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Combatusage:
                skill->occasion_battle = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Strengtheffect:
                skill->pdef_f = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Mindeffect:
                skill->mdef_f = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Variance:
                skill->variance = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Basevalue:
                skill->power = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Successrate:
                skill->hit = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectHP:
                skill->affect_hp = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectMP:
                skill->affect_sp = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectAttack:
                skill->affect_attack = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectDefense:
                skill->affect_defense = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectMind:
                skill->affect_spirit = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectAgility:
                skill->affect_agility = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Absorbdamage:
                skill->absorb_damage = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Ignoredefense:
                skill->ignore_defense = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Conditionslength:
                trash = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Changecondition:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    skill->condition_effects.push_back(Void);
                }
                break;
            case SkillChunk_Attributeslength:
                trash = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Attackattribute:
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    skill->attribute_effects.push_back(Void);
                }
                break;
			// What is this?
            /*case SkillChunk_Affectresistance:
                skill->affect_resistance = ReadCompressedInteger(Stream);
                break;*/
            case CHUNK_LDB_END_OF_BLOCK:
                break;
            default:
                while (ChunkInfo.Length--)
                {
                    bool return_value;
                    return_value = fread(&Void, 1, 1, Stream);
                }
                break;
            }
        } while (ChunkInfo.ID!=0);
        
        Main_Data::data_skills.push_back(skill);
        datareaded++;
    }
}
