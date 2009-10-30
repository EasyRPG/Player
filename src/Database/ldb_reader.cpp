#include "ldb_reader.h"

bool LDB_reader::load(std::string& Filename)
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
    sound_effect stcSound;
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
    sk->sound = stdSound;
    ChunkInfo.ID=1;
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

void LDB_reader::skillChunk(FILE * Stream)
{
    int id,datatoread=0,datareaded=0;
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
                skill->text = ReadString(Stream, ChunkInfo.Length);
                break;
            case SkillChunk_textline:
                skill->text2 = ReadString(Stream, ChunkInfo.Length);
                break;
            case SkillChunk_Failure:
                skill->failure_msg = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Spelltype:
                skill->spell_type = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Cost:
                skill->sp_cost = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Range:
                skill->scope = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Switch:
                skill->switch_on = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Battleanimation:
                skill->animation1_id = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Soundeffect:
                soundChunk(Stream, skill);
                break;
            case SkillChunk_Fieldusage:
                skill->field_usg = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Combatusage:
                skill->combat_usg = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Strengtheffect:
                skill->str_f = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Mindeffect:
                skill->int_f = ReadCompressedInteger(Stream);
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
                skill->affect_mp = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectAttack:
                skill->affect_str = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectDefense:
                skill->affect_pdef = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectMind:
                skill->affect_int = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_AffectAgility:
                skill->affect_agi = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Absorbdamage:
                skill->absorb_dmg = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Ignoredefense:
                skill->ignore_def = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Conditionslength:
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Changecondition:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    skill->change_condition.push_back(Void);
                }
                break;
            case SkillChunk_Attributeslength:
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case SkillChunk_Attackattribute:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    skill->attributes.push_back(Void);
                }
                break;
            case SkillChunk_Affectresistance:
                skill->affect_resistance = ReadCompressedInteger(Stream);
                break;
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
        
        Main_Data::data_skills->push_back(skill);
        datareaded++;
    }
}

void LDB_reader::itemChunk(FILE * Stream)
{
    int id,datatoread=0,datareaded=0;
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
                item->n_uses = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attack:
                item->atk_change = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Defense:
                item->pdef_change = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Mind:
                item->int_change = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Speed:
                item->agi_change = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Equip:
                item->both_hands = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPcost:
                item->mp_cost = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Chancetohit:
                item->hit = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Criticalhit:
                item->crit_hit = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Battleanimation:
                item->animation_id = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Preemptiveattack:
                item->preventive = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Doubleattack:
                item->double_attack = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attackallenemies:
                item->attack_all = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Ignoreevasion:
                item->ignore_evasion = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Preventcriticalhits:
                item->prevent_crits = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Raiseevasion:
                item->raise_evasion = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPusecutinhalf:
                item->half_mp = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Noterraindamage:
                item->no_terrain_dmg = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Healsparty:
                item->heals_party = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_HPrecovery ://0x20,
                item->recover_hp_rate = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_HPrecoveryvalue://0x21,
                item->recover_hp = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPrecovery://0x22,
                item->recover_mp_rate = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MPrecoveryvalue://0x23,
                item->recover_mp = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Useonfieldonly://0x25,
                item->field_only = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Onlyondeadheros://0x26,
                item->dead_only = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MaxHPmodify://0x29,
                item->max_hp_mod = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_MaxMPmodify://0x2A,
                item->max_mp_mod = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attackmodify://0x2B,
                item->str_mod = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Defensemodify://0x2C,
                item->pdef_mod = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Mindmodify://0x2D,
                item->int_mod = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Speedmodify://0x2E,
                item->agi_mod = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Usagemessage://0x33,
                item->use_msg = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Switchtoturnon://0x37,
                item->switch_on = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Useonfield://0x39,
                item->use_on_field = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Useinbattle://0x3A,
                item->use_in_battle = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Heroeslength://0x3D,
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Heroescanuse:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    item->heros_can_use.push_back(Void);
                }
                break;
            case ItemChunk_Conditionslength://0x3F,
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Conditionchanges:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    item->conditions.push_back(Void);
                }
                break;
            case ItemChunk_Attributeslength://0x41,
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case ItemChunk_Attributes:
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    item->attributes.push_back(Void);
                }
                break;
            case ItemChunk_Chancetochange://0x43
                item->chance_cond = ReadCompressedInteger(Stream);
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
        datareaded++;
        Main_Data::data_objects.push_back(item);
    }
}

void LDB_reader::mosteractionChunk(FILE * Stream, Enemy* e)
{
    int id,datatoread=0,datareaded=0;
    E_Action *action;
    datatoread=ReadCompressedInteger(Stream);//numero de datos
    while (datatoread>datareaded) 
    { // si no hay mas en el array
        action = new action();
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
                Action->skill_id = ReadCompressedInteger(Stream);
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
    int id,datatoread=0,datareaded=0;    

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
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Conditionseffects://0x20,
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    enemy->conditions.push_back(Void);
                }
                break;
            case MonsterChunk_Attributeslength://0x21,
                int trash;
                trash = ReadCompressedInteger(Stream);
                break;
            case MonsterChunk_Attributeseffect://0x22,
                bool return_value;
                while (ChunkInfo.Length--) {
                    return_value = fread(&Void, 1, 1, Stream);
                    enemy->attributes.push_back(Void);
                }
                break;
            case MonsterChunk_Actionslist://0x2A
                mosteractionChunk(Stream);
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
    stEvent Event_parser;
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
    int id,datatoread=0,datareaded=0;    
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

void LDB_reader::GetNextChunk(FILE * Stream)
{
    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido
    // Loop while we haven't reached the end of the file
    while (!feof(Stream) && (!stop)) {
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
                data->monsters = mosterChunk(Stream);
                break;
            case CHUNK_MonsterP:
                data->monsterpartys = mosterpartyChunk(Stream);
                break;
            case CHUNK_Terrain:
                data->terrains = terrainChunk(Stream);
                break;
            case CHUNK_Attribute:
                data->attributes = attributeChunk(Stream);
                break;
            case CHUNK_States:
                data->states = statesChunk(Stream);
                break;
            case CHUNK_Animation:
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
                break;
            default:
                // saltate un pedazo del tamaño de la longitud
                while (ChunkInfo.Length--) {
                    bool return_value;
                    return_value = fread(&Void, 1, 1, Stream);
                }
                break;
            }

    }
}