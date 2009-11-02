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
void LDB_reader::soundChunk(FILE * Stream, Skill* sk)// confusion masica != sonido
{
    RPG::Sound stcSound;
    do {
        ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
        if (ChunkInfo.ID!=0)
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

        switch (ChunkInfo.ID) { // segun el tipo
        case 0x01:
            stcSound.name	 = ReadString(Stream, ChunkInfo.Length);
            break;
        case 0x03:
            stcSound.volume	 = ReadCompressedInteger(Stream);
            break;
        case 0x04:
            stcSound.tempo	 = ReadCompressedInteger(Stream);
            break;
        case 0x05:
            stcSound.balance	 = ReadCompressedInteger(Stream);
            break;
        case 0x00:
            break;
        default:
            bool return_value;
            while (ChunkInfo.Length--) {
                return_value = fread(&Void, 1, 1, Stream);
            }
            break;
        }
    } while (ChunkInfo.ID!=0);
    sk->sound_effect = &stcSound;
    ChunkInfo.ID=1;
}

void LDB_reader::heroskillChunk(FILE * Stream, Actor* hero)
{
    int id,datatoread=0,datareaded=0;
    RPG::Learning skill;
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
    int datatoread=0,datareaded=0;
    int levels;
    short dat;
    Actor *hero_data;
    
    datatoread = ReadCompressedInteger(Stream);//numero de datos
    while (datatoread > datareaded) // si no hay mas en el array
    { 
        hero_data = new Actor();
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

void LDB_reader::skillChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    Skill *skill;
    datatoread=ReadCompressedInteger(Stream);//numero de datos

    while (datatoread>datareaded) 
    { // si no hay mas en el array
        skill = new Skill();
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
                soundChunk(Stream, skill);
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

void LDB_reader::itemChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    Item *item;
    datatoread=ReadCompressedInteger(Stream);
    while (datatoread>datareaded) 
    {
        item = new Item();
        if (item == NULL)
        {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }
        item->id = ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {            
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
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

void LDB_reader::mosteractionChunk(FILE * Stream, Enemy* e)
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

    Enemy *enemy;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    //printf("Numero de datos -> %d\n", datatoread);
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        enemy = new Enemy();
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
                enemy->crit_hits = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Criticalhitchance://0x16,
                enemy->crit_hits_chance = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Usuallymiss://0x1A,
                enemy->miss = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Airborne://0x1C,
                enemy->flying = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Conditionslength://0x1F,
                trash = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Conditionseffects://0x20,
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    enemy->conditions.push_back(Void);
                }
                break;
            case MonsterChunk_Attributeslength://0x21,
                trash = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Attributeseffect://0x22,
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    enemy->attributes.push_back(Void);
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

void LDB_reader::MonsterPartyMonsterChunk(FILE * Stream, Troop *trp)
{
    int id,datatoread=0,datareaded=0;
    Member *monster = new Member();
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

void LDB_reader::MonsterPartyEventconditionChunk(FILE * Stream, Page* pgs)
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

void LDB_reader::MonsterPartyevent_pageChunk(FILE * Stream, Troop *trp)
{
    int id,datatoread=0,datareaded=0;
    Page Monsterevent;
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
    Troop *enemy_group;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        enemy_group = new Troop();
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

void LDB_reader::terrainChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    std::string name;
    Terrain *terrain;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        terrain = new Terrain();
        if (terrain == NULL)
        {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }   
        terrain->id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do 
        {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
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

void LDB_reader::attributeChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;

    Attribute *attribute;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        attribute = new Attribute();
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
                attribute->A_damage = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_B_damage://0x0C,
                attribute->B_damage = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_C_damage://0x0F,
                attribute->C_damage = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_D_damage://0x0F,
                attribute->D_damage = ReadCompressedInteger(Stream);
                break;
            case AttributeChunk_E_damage://0x0F
                attribute->E_damage = ReadCompressedInteger(Stream);
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

void LDB_reader:: statesChunk(FILE * Stream)
{
    int datatoread=0,datareaded=0;
    State *state;
    
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        state = new State();
        if (state == NULL) {
            std::cerr << "No memory left." << std::endl;
            exit(1);
        }
        state->id= ReadCompressedInteger(Stream);//lectura de id 1 de array
        do {
            ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if (ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            switch (ChunkInfo.ID) { // tipo de la primera dimencion
            case StatesChunk_Name:
                state->name = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Length://0x02,
                state->battle_only = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Color://0x03,
                state->color = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Priority://0x04,
                state->priority = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Limitation://0x05,
                state->restriction = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_A_chance://0x0B,
                state->A_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_B_chance://0x0C,
                state->B_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_C_chance://0x0D,
                state->C_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_D_chance://0x0E,
                state->D_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_E_chance://0x0F,
                state->E_chance = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Turnsforhealing://0x15,
                state->hold_turn = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Healperturn://0x16,
                state->auto_release_prob = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Healonshock://0x17,
                state->shock_release_prob = ReadCompressedInteger(Stream);
                break;
            case 0x1E: //0: reducir a la mitad 1: doble 2: No cambiar
                state->type_change = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveAttack://0x1F,
                state->atk_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveDefense://0x20,
                state->pdef_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveMind://0x21,
                state->int_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HalveAgility://0x22,
                state->agi_mod = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Hitratechange://0x23,
                state->rate_change = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Preventskilluse://0x29,
                state->prevent_skill_use = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Minimumskilllevel://0x2A,
                state->min_skill_lvl = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Preventmagicuse://0x2B,
                state->prevent_magic_use = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Minimummindlevel://0x2C,
                state->min_magic_lvl = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_Allyenterstate://0x33,
                state->ally_enter_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Enemyentersstate://0x34,
                state->enemy_enter_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Alreadyinstate://0x35,
                state->already_in_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Affectedbystate://0x36,
                state->affected_by_state = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_Statusrecovered://0x37,
                state->status_recovered = ReadString(Stream, ChunkInfo.Length);
                break;
            case StatesChunk_HPloss://0x3D,
                state->hp_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HPlossvalue://0x3E,
                state->hp_loss_value = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HPmaploss://0x3F,
                state->hp_map_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_HPmapsteps://0x40,
                state->hp_map_steps = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPloss://0x41,
                state->mp_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPlossvalue://0x42,
                state->mp_loss_value = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPmaploss://0x43,
                state->mp_map_loss = ReadCompressedInteger(Stream);
                break;
            case StatesChunk_MPmapsteps://0x44
                state->mp_map_steps = ReadCompressedInteger(Stream);
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
        Main_Data::data_states.push_back(state);
    }
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
            /*case CHUNK_Animation:
                data->animations = animationChunk(Stream);
                break;
            case CHUNK_Tileset:
                data->tilesets = tilesetChunk(Stream);
                break;
            case CHUNK_String:
                data->Glosary = stringChunk(Stream);
                break;
            case CHUNK_System:
                data->System_dat = systemChunk(Stream);
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
