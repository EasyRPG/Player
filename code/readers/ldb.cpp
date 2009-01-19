// =========================================================================
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "../tools/tools.h"
#include "ldb.h"
#include "stevent.h"
#include <iostream>
// =========================================================================

// --- Create list of chunks opcodes ---------------------------------------

bool stop = false;

bool  LDB_reader::Load(std::string Filename, LDB_data * data)
{

	// Open map file to read
	FILE * Stream;// apertura de archivo
	Stream = fopen(Filename.c_str(), "rb");
	if (Stream == NULL)
	{
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
	GetNextChunk(Stream,data);

	fclose(Stream);
	return true;
}

std::vector <Magicblock>  LDB_reader::heroskillChunk(FILE * Stream)
{
	int id,datatoread=0,datareaded=0;
	Magicblock skill;
	std::vector <Magicblock> vecSkills;
	datatoread = ReadCompressedInteger(Stream); //numero de datos
	while(datatoread>datareaded) //si no hay mas en el array
	{
		id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		do
		{
			ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
			switch(ChunkInfo.ID)// tipo de la primera dimencion
			{
				case MagicblockChunk_Level:
					skill.Level = ReadCompressedInteger(Stream);
					break;
				case MagicblockChunk_Spell_ID:
					skill.Spell_ID = ReadCompressedInteger(Stream);
					break;
				case CHUNK_LDB_END_OF_BLOCK:
					break;
				default:
					while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
					break;
			}
		} while(ChunkInfo.ID!=0);
		vecSkills.push_back(skill);
		skill.clear();
		datareaded++;
	}
	ChunkInfo.ID=1;
	return(vecSkills);
}


std::vector <stcHero>  LDB_reader::heroChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0,comands=0;
		 int levels;
		 short dat;
		 std:: vector <stcHero> vecHero;
		 stcHero hero;
		 hero.clear();
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		  do{
			  ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			  //printf("%x\n", ChunkInfo.ID);
			  if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			  ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				switch(ChunkInfo.ID)// tipo de la primera dimencion
				{
				case CHUNK_Name:
				     hero.strName = ReadString(Stream, ChunkInfo.Length);
				     break;
				case CHUNK_Class:
				     hero.strClass = ReadString(Stream, ChunkInfo.Length);
				     break;				case CHUNK_Graphicfile:
				     hero.strGraphicfile = ReadString(Stream, ChunkInfo.Length);

				     break;
				case CHUNK_Graphicindex:				     hero.intGraphicindex = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Transparent:
				     hero.intTransparent = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Startlevel:
				     hero.intStartlevel = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Maxlevel:
				     hero.intMaxlevel = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Crithit:
				     hero.intCrithit = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Hitchance:
				     hero.intHitchance = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Facegraphic:
				     hero.strFacegraphic = ReadString(Stream, ChunkInfo.Length);
				     break;
				case CHUNK_Faceindex:
				     hero.intFaceindex = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Dualwield:
				     hero.blDualwield = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Fixedequipment:
				     hero.blFixedequipment = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_AI:
				     hero.blAI = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Highdefense:
				     hero.blHighdefense = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Statisticscurves:

						 levels=ChunkInfo.Length/6;
						 while(levels > 0)
						 {
                      fread(&dat, 2, 1, Stream);
						    hero.vc_sh_Hp.push_back(dat);
						    levels-=2;
						 }

						 levels=ChunkInfo.Length/6;
						 while(levels > 0)
						 {
						    fread(&dat, 2, 1, Stream);
						    hero.vc_sh_Mp.push_back(dat);
						    levels-=2;
						 }

						 levels=ChunkInfo.Length/6;
						 while(levels > 0)
						 {
						    fread(&dat, 2, 1, Stream);
						    hero.vc_sh_Attack.push_back(dat);						    levels-=2;
						 }

						 levels=ChunkInfo.Length/6;
						 while(levels > 0)
						 {
						    fread(&dat, 2, 1, Stream);
						    hero.vc_sh_Defense.push_back(dat);
						    levels-=2;
						 }

						 levels=ChunkInfo.Length/6;
						 while(levels > 0)
						 {
						    fread(&dat, 2, 1, Stream);
						    hero.vc_sh_Mind.push_back(dat);
						    levels-=2;
						 }

						 levels=ChunkInfo.Length/6;
						 while(levels > 0)
						 {
						    fread(&dat, 2, 1, Stream);
						    hero.vc_sh_Agility.push_back(dat);
						    levels-=2;
						 }
						 break;
				case CHUNK_EXPBaseline:
				     hero.intEXPBaseline = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_EXPAdditional:
				     hero.intEXPAdditional = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_EXPCorrection:
				     hero.intEXPCorrection = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Startequip:
							   fread(&dat, sizeof(short), 1, Stream);
							   hero.sh_Weapon = dat;
							   fread(&dat, sizeof(short), 1, Stream);
							   hero.sh_Shield = dat;
							   fread(&dat, sizeof(short), 1, Stream);
							   hero.sh_Armor = dat;
							   fread(&dat, sizeof(short), 1, Stream);
							   hero.sh_Head = dat;
							   fread(&dat, sizeof(short), 1, Stream);
							   hero.sh_Accessory = dat;
				    	       break;
				case CHUNK_Skills:
				     hero.skills = heroskillChunk(Stream);
				     break;
				case CHUNK_RenameMagic:
				     hero.blRenameMagic = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Magicname:
				     hero.strMagicname = ReadString(Stream, ChunkInfo.Length);
				     break;
				case CHUNK_Condlength:
				     hero.intCondlength = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Condeffects:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								hero.vc_ch_Condeffects.push_back(Void);
								}
								break;
				case CHUNK_Attriblength:
				     hero.intAttriblength = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_Attribeffects:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								hero.vc_ch_Attribeffects.push_back(Void);
								}
								break;
				case CHUNK_Combat_Command://0x50
							while(ChunkInfo.Length--) //4 chars
							{fread(&comands, sizeof(int), 1, Stream);
							hero.vc_int_Combat_Command.push_back(comands);
							ChunkInfo.Length-=3;}
							break;
				case CHUNK_LDB_END_OF_BLOCK:
				     break;
						  default:
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			}while(ChunkInfo.ID!=0);
			vecHero.push_back(hero);
			hero.clear();
		  datareaded++;
		  }
		  return(vecHero);
}
stcSound_effect  LDB_reader::soundChunk(FILE * Stream)// confusion masica != sonido
{
stcSound_effect  stcSound;
stcSound.clear();
do
 {
			ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			if(ChunkInfo.ID!=0)
			ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

			switch(ChunkInfo.ID)// segun el tipo
			{
			   case 0x01:
					 stcSound.Name_of_Sound_effect	 = ReadString(Stream, ChunkInfo.Length);
					break;
			   case 0x03:
					 stcSound.Volume	 = ReadCompressedInteger(Stream);
					break;
			   case 0x04:
					 stcSound.Tempo	 = ReadCompressedInteger(Stream);
					break;
			   case 0x05:
					 stcSound.Balance	 = ReadCompressedInteger(Stream);
					break;
			   case 0x00:
					break;
			   default:
					while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
					break;
			}
 }while (ChunkInfo.ID!=0);
   ChunkInfo.ID=1;
 return(stcSound);
}

std:: vector <stcSkill>  LDB_reader::skillChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std:: vector <stcSkill> vecSkill;
		 stcSkill skill;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos

		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			  do{
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				  ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				switch(ChunkInfo.ID)// tipo de la primera dimencion
				{
						  case SkillChunk_Name:
							   skill.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						  case SkillChunk_Description:
							   skill.strDescription = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case SkillChunk_text:
							   skill.strtext = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case SkillChunk_textline:
							   skill.strtextline = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case SkillChunk_Failure:
							   skill.intFailure = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Spelltype:
							   skill.intSpelltype = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Cost:
							   skill.intCost = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Range:
							   skill.intRange = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Switch:
							   skill.intSwitch = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Battleanimation:
							   skill.intBattleanimation = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Soundeffect:
							   skill.Soundeffect = soundChunk(Stream);
							   break;
						  case SkillChunk_Fieldusage:
							   skill.blFieldusage = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Combatusage:
							   skill.blCombatusage = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Strengtheffect:
							   skill.intStrengtheffect = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Mindeffect:
							   skill.intMindeffect = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Variance:
							   skill.intVariance = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Basevalue:
							   skill.intBasevalue = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Successrate:
							   skill.intSuccessrate = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_AffectHP:
							   skill.blAffectHP = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_AffectMP:
							   skill.blAffectMP = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_AffectAttack:
							   skill.blAffectAttack = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_AffectDefense:
							   skill.blAffectDefense = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_AffectMind:
							   skill.blAffectMind = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_AffectAgility:
							   skill.blAffectAgility = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Absorbdamage:
							   skill.blAbsorbdamage = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Ignoredefense:
							   skill.blIgnoredefense = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Conditionslength:
							   skill.intConditionslength = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Changecondition:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								skill.vc_ch_Condeffects.push_back(Void);
								}
								break;
						  case SkillChunk_Attributeslength:
							   skill.intAttributeslength = ReadCompressedInteger(Stream);
							   break;
						  case SkillChunk_Attackattribute:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								skill.vc_ch_Attribeffects.push_back(Void);
								}
								break;
						  case SkillChunk_Affectresistance:
							   skill.blAffectresistance = ReadCompressedInteger(Stream);
						       break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
						  default:
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			} while(ChunkInfo.ID!=0);
		  vecSkill.push_back(skill);
		  skill.clear();
		  datareaded++;

		  }
		  return(vecSkill);
}
std:: vector <stcItem>  LDB_reader::itemChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std::string name;
		 std::vector <stcItem> vecItem;
		 stcItem Item;
		 datatoread=ReadCompressedInteger(Stream);
		 while(datatoread>datareaded)
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			  do{
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				switch(ChunkInfo.ID)// tipo de la primera dimencion
				{
						  case ItemChunk_Name:
							   Item.Name = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case ItemChunk_Description:
							   Item.Description = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case ItemChunk_Type:
							   Item.Type = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Cost:
							   Item.Cost = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Uses:
							   Item.Uses = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Attack:
							   Item.Attack = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Defense:
							   Item.Defense = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Mind:
							   Item.Mind = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Speed:
							   Item.Speed = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Equip:
							   Item.Equip = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_MPcost:
							   Item.MPcost = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Chancetohit:
							   Item.Chancetohit = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Criticalhit:
							   Item.Criticalhit = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Battleanimation:
							   Item.Battleanimation = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Preemptiveattack:
							   Item.Preemptiveattack = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Doubleattack:
							   Item.Doubleattack = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Attackallenemies:
							   Item.Attackallenemies = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Ignoreevasion:
							   Item.Ignoreevasion = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Preventcriticalhits:
							   Item.Preventcriticalhits = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Raiseevasion:
							   Item.Raiseevasion = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_MPusecutinhalf:
							   Item.MPusecutinhalf = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Noterraindamage:
							   Item.Noterraindamage = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Healsparty:
							   Item.Healsparty = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_HPrecovery ://0x20,
							   Item.HPrecovery = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_HPrecoveryvalue://0x21,
							   Item.HPrecoveryvalue = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_MPrecovery://0x22,
							   Item.MPrecovery = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_MPrecoveryvalue://0x23,
							   Item.MPrecoveryvalue = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Useonfieldonly://0x25,
							   Item.Useonfieldonly = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Onlyondeadheros://0x26,
							   Item.Onlyondeadheros = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_MaxHPmodify://0x29,
							   Item.MaxHPmodify = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_MaxMPmodify://0x2A,
							   Item.MaxMPmodify = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Attackmodify://0x2B,
							   Item.Attackmodify = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Defensemodify://0x2C,
							   Item.Defensemodify = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Mindmodify://0x2D,
							   Item.Mindmodify = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Speedmodify://0x2E,
							   Item.Speedmodify = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Usagemessage://0x33,
							   Item.Usagemessage = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Switchtoturnon://0x37,
							   Item.Switchtoturnon = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Useonfield://0x39,
							   Item.Useonfield = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Useinbattle://0x3A,
							   Item.Useinbattle = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Heroeslength://0x3D,
							   Item.Heroeslength = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Heroescanuse:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								Item.vc_ch_Heroescanuse.push_back(Void);
								}
								break;
						  case ItemChunk_Conditionslength://0x3F,
							   Item.Conditionslength = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Conditionchanges:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								Item.vc_ch_Condeffects.push_back(Void);
							    }							break;
						  case ItemChunk_Attributeslength://0x41,
							   Item.Attributeslength = ReadCompressedInteger(Stream);
							   break;
						  case ItemChunk_Attributes:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								Item.vc_ch_Attribeffects.push_back(Void);
								}
								break;
						  case ItemChunk_Chancetochange://0x43
						       Item.Chancetochange = ReadCompressedInteger(Stream);							break;						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
						  default:
					                while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			 } while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecItem.push_back(Item);
		  Item.clear();
		  }
		  return(vecItem);
}

std:: vector <stcEnemy_Action>  LDB_reader::mosteractionChunk(FILE * Stream)
{
 int id,datatoread=0,datareaded=0;
 stcEnemy_Action Action;
std::vector <stcEnemy_Action> vecActions;
	datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		  do{
			  ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			  if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			  ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case	MonsterActionsChunk_Action:
							   Action.intAction = ReadCompressedInteger(Stream);
							    break;
						  case MagicblockChunk_Spell_ID:
							   Action.intAction_data = ReadCompressedInteger(Stream);
							   break;
						  case MonsterActionsChunk_Skill_ID:
							   Action.intSkill_ID = ReadCompressedInteger(Stream);
							   break;
						  case MonsterActionsChunk_Monster_ID:
							   Action.intMonster_ID = ReadCompressedInteger(Stream);
							   break;
						  case MonsterActionsChunk_Condition:
							   Action.intCondition = ReadCompressedInteger(Stream);
							   break;
						  case MonsterActionsChunk_Lower_limit:
							   Action.intLower_limit = ReadCompressedInteger(Stream);
							   break;
						  case MonsterActionsChunk_Upper_limit:
							   Action.intUpper_limit = ReadCompressedInteger(Stream);
							   break;
						  case MonsterActionsChunk_Priority:
							   Action.intPriority = ReadCompressedInteger(Stream);
							   break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
						  default:
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			}while(ChunkInfo.ID!=0);
			vecActions.push_back(Action);
			Action.clear();
		  datareaded++;
		  }
		  ChunkInfo.ID	 =1;
		  return(vecActions);
}

std:: vector <stcEnemy>  LDB_reader::mosterChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std:: vector <stcEnemy> vecEnemy;
		 stcEnemy Enemy;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 //printf("Numero de datos -> %d\n", datatoread);
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			  do{
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				//printf("%x\n", ChunkInfo.ID);
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case MonsterChunk_Name:
								Enemy.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						  case MonsterChunk_Graphicfile:
							   Enemy.strGraphicfile = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case MonsterChunk_Huealteration://0x03,
							   Enemy.intHuealteration = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_MaxHP://0x04,
							   Enemy.intMaxHP = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_MaxMP://0x05,
							   Enemy.intMaxMP = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Attack://0x06,
							   Enemy.intAttack = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Defense://0x07,
							   Enemy.intDefense = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Mind://0x08,
							   Enemy.intMind = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Speed://0x09,
							   Enemy.intSpeed = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Translucentgraphic://0x0A,
							   Enemy.blTranslucentgraphic = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Experience://0x0B,
							   Enemy.intExperience = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Gold://0x0C,
							   Enemy.intGold = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_SpoilsitemID://0x0D,
							   Enemy.intSpoilsitemID = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Spoilschance://0x0E,
							   Enemy.intSpoilschance = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Canusecriticalhits://0x15,
							   Enemy.blCanusecriticalhits = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Criticalhitchance://0x16,
							   Enemy.intCriticalhitchance = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Usuallymiss://0x1A,
							   Enemy.blUsuallymiss = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Airborne://0x1C,
							   Enemy.blAirborne = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Conditionslength://0x1F,
							   Enemy.intConditionslength = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Conditionseffects://0x20,
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								Enemy.vc_ch_Condeffects.push_back(Void);
								}
								break;
						  case MonsterChunk_Attributeslength://0x21,
							   Enemy.inyAttributeslength = ReadCompressedInteger(Stream);
							   break;
						  case MonsterChunk_Attributeseffect://0x22,
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								Enemy.vc_ch_Attribeffects.push_back(Void);
								}
								break;
						  case MonsterChunk_Actionslist://0x2A
							   mosteractionChunk(Stream);
							   break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			} while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecEnemy.push_back(Enemy);
		  Enemy.clear();
		  }
		  //stop = true;
		  return(vecEnemy);
}

std:: vector <stcEnemy_group_data>  LDB_reader::MonsterPartyMonsterChunk(FILE * Stream)
{
 int id,datatoread=0,datareaded=0;
 stcEnemy_group_data Monster;
std::vector <stcEnemy_group_data> vecPartyMonster;
	datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		  do{
			  ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			  if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			  ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case	Monster_ID:
							   Monster.Enemy_ID = ReadCompressedInteger(Stream);
							   break;
						  case X_position:
							   Monster.X_position = ReadCompressedInteger(Stream);
							   break;
						  case Y_position:
							   Monster.Y_position = ReadCompressedInteger(Stream);
							   break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
						  default:
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			}while(ChunkInfo.ID!=0);
			vecPartyMonster.push_back(Monster);
			Monster.clear();
		  datareaded++;
		  }
		  ChunkInfo.ID	 =1;
		  return(vecPartyMonster);
}




stcEnemy_group_condition   LDB_reader::MonsterPartyEventconditionChunk(FILE * Stream)
{
  stcEnemy_group_condition  stcCondition;
  short dat;
  stcCondition.clear();
do
 {
			ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			if(ChunkInfo.ID!=0)
			ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

			switch(ChunkInfo.ID)// segun el tipo
			{
			   case Condition_flags:// en el 2003 se usan 2 chars
				   if(ChunkInfo.Length==2)
				   stcCondition.Condition_flags =fread(&dat, sizeof(short), 1, Stream);
					else
					stcCondition.Condition_flags = ReadCompressedInteger(Stream);
				    break;
			   case Switch_A:
					stcCondition.Switch_A	 = ReadCompressedInteger(Stream);
					break;
			   case Turn_number_A:
					 stcCondition.Turn_number_A	 = ReadCompressedInteger(Stream);
					break;
			   case Lower_limit:
					 stcCondition.Lower_limit	 = ReadCompressedInteger(Stream);
					break;
			   case Upper_limit:
					 stcCondition.Upper_limit	 = ReadCompressedInteger(Stream);
					break;
			   case CHUNK_LDB_END_OF_BLOCK:
					break;
			   default:
					while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
					break;
			}
 }while (ChunkInfo.ID!=0);
   ChunkInfo.ID=1;
 return(stcCondition);
}


std:: vector <stcEnemy_group_event_page>  LDB_reader::MonsterPartyevent_pageChunk(FILE * Stream)
{
 int id,datatoread=0,datareaded=0;
 stcEnemy_group_event_page Monsterevent;
 std::vector <stcEnemy_group_event_page> vecPartyMonsterevent;
 stEvent Event_parser;
 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		  do{
			  ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			  if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			  ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
			  switch(ChunkInfo.ID)// tipo de la primera dimencion
			  {
				case Page_conditions:
				     Monsterevent.conditions=MonsterPartyEventconditionChunk(Stream);
	  			     break;
				case Event_length:
				     Monsterevent.Event_length = ReadCompressedInteger(Stream);
				     break;
				case Event_Monster:
				  /*printf("\n Monster event ");
				  while(ChunkInfo.Length--)
					{ fread(&Void, sizeof(char), 1, Stream);
					printf(" %d",Void);
}*/

  Monsterevent.vcEvent_comand =  Event_parser.EventcommandChunk(Stream);
                               	     break;
				case CHUNK_LDB_END_OF_BLOCK:
				       break;
				default:
					while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
					break;
						  }
			}while(ChunkInfo.ID!=0);
			vecPartyMonsterevent.push_back(Monsterevent);
			Monsterevent.clear();
		  datareaded++;
		  }
		  ChunkInfo.ID	 =1;
		  return(vecPartyMonsterevent);
}



std:: vector <stcEnemy_group>  LDB_reader::mosterpartyChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std:: vector <stcEnemy_group> vecEnemy_group;
		 stcEnemy_group Enemy_group;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			 do{
				ChunkInfo.ID	 = ReadCompressedInteger(Stream);
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); 					     				        switch(ChunkInfo.ID)// tipo de la primera dimencion
				{
				case MonsterPartyChunk_Name:
					Enemy_group.strName = ReadString(Stream, ChunkInfo.Length);
								break;
				case MonsterPartyChunk_Monsterdata://0x02,
				Enemy_group.Enemy_data =MonsterPartyMonsterChunk(Stream);
							   break;
				case MonsterPartyChunk_Terrainlength://0x04,
							   Enemy_group.intTerrainlength = ReadCompressedInteger(Stream);
							  break;
				case MonsterPartyChunk_Terraindata://0x05,
								while(ChunkInfo.Length--)
								{
								fread(&Void, sizeof(char), 1, Stream);
								Enemy_group.vc_ch_Terraindata.push_back(Void);
								}
							   break;
				case MonsterPartyChunk_eventpages://0x0B
				Enemy_group.vecPartyMonsterevent=MonsterPartyevent_pageChunk(Stream);
							   break;
				case CHUNK_LDB_END_OF_BLOCK:
							   break;
				default:
				   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
				}
		  }while(ChunkInfo.ID!=0);
		  datareaded++;
		 vecEnemy_group.push_back(Enemy_group);
		 Enemy_group.clear();
		  }
		  return(vecEnemy_group);
}

std:: vector <stcTerrain>  LDB_reader:: TerrainChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std::string name;
		 std::vector <stcTerrain> vecTerrain;
		 stcTerrain Terrain;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		    do{
			ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
			switch(ChunkInfo.ID)// tipo de la primera dimencion
			{
				case TerrainChunk_Name:
					Terrain.strName = ReadString(Stream, ChunkInfo.Length);
				     break;
				case TerrainChunk_Damageontravel://0x02,
							   Terrain.intDamageontravel = ReadCompressedInteger(Stream);
							   break;
				case TerrainChunk_Encounterate://0x03,
							   Terrain.intEncounterate = ReadCompressedInteger(Stream);
							   break;
				case TerrainChunk_Battlebackground://0x04,
				     Terrain.strBattlebackground = ReadString(Stream, ChunkInfo.Length);
							   break;
				case TerrainChunk_Skiffmaypass://0x05,
							   Terrain.blSkiffmaypass = ReadCompressedInteger(Stream);
							   break;
				case TerrainChunk_Boatmaypass://0x06,
							   Terrain.blBoatmaypass = ReadCompressedInteger(Stream);
							   break;
				case TerrainChunk_Airshipmaypass://0x07,
							   Terrain.blAirshipmaypass = ReadCompressedInteger(Stream);
							   break;
				case TerrainChunk_Airshipmayland://0x09,
							   Terrain.blAirshipmayland = ReadCompressedInteger(Stream);
							   break;
				case TerrainChunk_Heroopacity://0x0B
					Terrain.intHeroopacity = ReadCompressedInteger(Stream);
					break;
				case CHUNK_LDB_END_OF_BLOCK:
					break;
				default:
				      while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			  }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecTerrain.push_back(Terrain);
		 Terrain.clear();
		  }
		  return(vecTerrain);
}

std:: vector <stcAttribute>  LDB_reader:: AttributeChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std::string name;
		 std::vector <stcAttribute> vecAttribute;
		 stcAttribute Attribute;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			  do{
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				 ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case AttributeChunk_Name:
								 Attribute.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						 case AttributeChunk_Type://0x02,
							   Attribute.intType = ReadCompressedInteger(Stream);
							   break;
						 case AttributeChunk_A_damage://0x0B,
							   Attribute.intA_damage = ReadCompressedInteger(Stream);
							   break;
						 case AttributeChunk_B_damage://0x0C,
							   Attribute.intB_damage = ReadCompressedInteger(Stream);
							   break;
						 case AttributeChunk_C_damage://0x0F,
							   Attribute.intC_damage = ReadCompressedInteger(Stream);
							   break;
						 case AttributeChunk_D_damage://0x0F,
							   Attribute.intD_damage = ReadCompressedInteger(Stream);
							   break;
						 case AttributeChunk_E_damage://0x0F
							   Attribute.intE_damage = ReadCompressedInteger(Stream);
							   break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }

			  }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecAttribute.push_back(Attribute);
		  Attribute.clear();
		  }
		  return(vecAttribute);
}

std:: vector <stcState>  LDB_reader:: StatesChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std::string name;
		 std:: vector <stcState> vecState;
		 stcState State;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		 do{
			ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case StatesChunk_Name:
							   State.strName = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case StatesChunk_Length://0x02,
							   State.intLength = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Color://0x03,
							   State.intColor = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Priority://0x04,
							   State.intPriority = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Limitation://0x05,
							   State.intLimitation = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_A_chance://0x0B,
							   State.intA_chance = ReadCompressedInteger(Stream);
							  break;
						  case StatesChunk_B_chance://0x0C,
							   State.intB_chance = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_C_chance://0x0D,
							   State.intC_chance = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_D_chance://0x0E,
							   State.intD_chance = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_E_chance://0x0F,
								State.intE_chance = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Turnsforhealing://0x15,
								State.intTurnsforhealing = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Healperturn://0x16,
								State.intHealperturn = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Healonshock://0x17,
							   State.intHealonshock = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_HalveAttack://0x1F,
							   State.blHalveAttack = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_HalveDefense://0x20,
							   State.blHalveDefense = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_HalveMind://0x21,
							   State.blHalveMind = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_HalveAgility://0x22,
							   State.blHalveAgility = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Hitratechange://0x23,
							   State.intHitratechange = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Preventskilluse://0x29,
							   State.blPrevent_skill_use = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Minimumskilllevel://0x2A,
							   State.intMinimum_skill_level = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Preventmagicuse://0x2B,
							   State.blPreventmagicuse = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Minimummindlevel://0x2C,
							   State.intMinimummindlevel = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_Allyenterstate://0x33,
							  State.strAllyenterstate = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case StatesChunk_Enemyentersstate://0x34,
							   State.strEnemyentersstate = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case StatesChunk_Alreadyinstate://0x35,
							   State.strAlreadyinstate = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case StatesChunk_Affectedbystate://0x36,
							   State.strAffectedbystate = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case StatesChunk_Statusrecovered://0x37,
							   State.strStatusrecovered = ReadString(Stream, ChunkInfo.Length);
							   break;
						  case StatesChunk_HPloss://0x3D,
							   State.intHPloss = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_HPlossvalue://0x3E,
							   State.intHPlossvalue = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_HPmaploss://0x3F,
							   State.intHPmaploss = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_HPmapsteps://0x40,
							   State.intHPmapsteps = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_MPloss://0x41,
								State.intMPloss = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_MPlossvalue://0x42,
							   State.intMPlossvalue = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_MPmaploss://0x43,
							   State.intMPmaploss = ReadCompressedInteger(Stream);
							   break;
						  case StatesChunk_MPmapsteps://0x44
							   State.intMPmapsteps = ReadCompressedInteger(Stream);
							   break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
		   }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecState.push_back(State);
		  State.clear();
		  }
		  return(vecState);
}

std:: vector <stcAnimationTiming>  LDB_reader:: AnimationTimingChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 stcAnimationTiming AnimationTiming;
		 std:: vector <stcAnimationTiming> vecAnimationTiming;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		 do{
		    ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
		    if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
		     ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
		    switch(ChunkInfo.ID)// tipo de la primera dimencion
			{
			case Frame://0x01,
			     AnimationTiming.Frame=ReadCompressedInteger(Stream);
			     break;
			case Sound_effect://0x02,
			AnimationTiming.Sound_effect = soundChunk(Stream);
			     break;
			case Flash_effect://0x03,
				AnimationTiming.Frame=ReadCompressedInteger(Stream);
				break;
			case Green_component://0x05,
				AnimationTiming.Frame=ReadCompressedInteger(Stream);
				break;
			case Blue_component://0x06,
				AnimationTiming.Frame=ReadCompressedInteger(Stream);
				break;
			case Flash_power://0x07
				AnimationTiming.Frame=ReadCompressedInteger(Stream);
				break;
			case CHUNK_LDB_END_OF_BLOCK:
			     break;
			default:
			     while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
			     break;
						  }
		  }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecAnimationTiming.push_back(AnimationTiming);
		  AnimationTiming.clear();
		  }
		  ChunkInfo.ID=1;
		  return(vecAnimationTiming);
}

std:: vector <stcAnimationCelldata>  LDB_reader:: AnimationCelldataChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 stcAnimationCelldata Celldata;
		 std:: vector <stcAnimationCelldata> vecCelldata;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		 do{
			   ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			   if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			   ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case AnimationCelldata_Cell_source://0x02,
							   Celldata.Cell_source = ReadCompressedInteger(Stream);
							   break;
						  case AnimationCelldata_X_location://0x03,
							   Celldata.X_location = ReadCompressedInteger(Stream);
							   break;
						  case AnimationCelldata_Y_location://0x04,
							   Celldata.Y_location = ReadCompressedInteger(Stream);
							   break;
						  case AnimationCelldata_Magnification://0x05,
							   Celldata.Magnification = ReadCompressedInteger(Stream);
							   break;
						  case AnimationCelldata_Red_component://0x06,
							   Celldata.Red_component = ReadCompressedInteger(Stream);
							   break;
						  case AnimationCelldata_Green_component://0x07,
							   Celldata.Green_component = ReadCompressedInteger(Stream);
							   break;
						  case AnimationCelldata_Blue_component://0x08,
							   Celldata.Blue_component = ReadCompressedInteger(Stream);
							   break;
						  case AnimationCelldata_Chroma_component://0x09,
							   Celldata.Chroma_component = ReadCompressedInteger(Stream);
							  break;
						  case AnimationCelldata_Alpha://0x0A,
							   Celldata.Alpha = ReadCompressedInteger(Stream);
							   break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
						  default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
		  }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecCelldata.push_back(Celldata);
		  Celldata.clear();
		  }
		  ChunkInfo.ID=1;
		  return(vecCelldata);
}

std:: vector <stcAnimationCell>  LDB_reader:: FramedataChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 stcAnimationCell AnimationCell;
		 std:: vector <stcAnimationCell> vecAnimationCell;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		 do{
			   ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			   if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			   ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
			   switch(ChunkInfo.ID)// tipo de la primera dimencion
				{
				case Cell_data:
				     AnimationCell.Cell_data=AnimationCelldataChunk(Stream);
					break;
				case CHUNK_LDB_END_OF_BLOCK:
					break;
				default:
				     while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
					break;
						  }
		  }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecAnimationCell.push_back(AnimationCell);
		  AnimationCell.clear();
		  }
		  ChunkInfo.ID=1;
		  return(vecAnimationCell);
}

std:: vector <stcAnimated_battle>  LDB_reader:: AnimationChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 stcAnimated_battle Animated_battle;
		 std:: vector <stcAnimated_battle> vecAnimated_battle;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		 do{
			   ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			   if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			   ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
			   switch(ChunkInfo.ID)// tipo de la primera dimencion
				{
				case AnimationChunk_Name:
					Animated_battle.strName = ReadString(Stream, ChunkInfo.Length);
				     break;
				case AnimationChunk_Animation_file://=0x02,
					Animated_battle.strAnimation_file = ReadString(Stream, ChunkInfo.Length);
								break;
				case AnimationChunk_Timing_data://=0x06,
				     Animated_battle.vecAnimationTiming= AnimationTimingChunk(Stream);
				    break;
				case AnimationChunk_Apply_to://=0x09,
				     Animated_battle.intApply_to = ReadCompressedInteger(Stream);
				     break;
				case AnimationChunk_Y_coordinate_line://=0x0A,
				     Animated_battle.intY_coordinate_line = ReadCompressedInteger(Stream);
				    break;
				case AnimationChunk_Framedata://=0x0C
				     Animated_battle.Framedata= FramedataChunk(Stream);
				     break;
				case CHUNK_LDB_END_OF_BLOCK:
				     break;
				default:
				     while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
				     break;
						  }
		  }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecAnimated_battle.push_back(Animated_battle);
		  Animated_battle.clear();
		  }
		  return(vecAnimated_battle);
}
std:: vector <stcChipSet>  LDB_reader:: TilesetChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 short dat=0;
		 stcChipSet ChipSet;
		 std:: vector <stcChipSet> vecChipset;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		 do{
				   ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				   if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				   ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case TilesetChunk_Name:
								ChipSet.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						  case TilesetChunk_Graphic://0x02,
								ChipSet.strGraphic = ReadString(Stream, ChunkInfo.Length);
								break;
						  case TilesetChunk_Lower_tile_terrain://0x03,
								while(ChunkInfo.Length--)
								{fread(&dat, sizeof(short), 1, Stream);
								ChipSet.vc_sh_Lower_tile_terrain.push_back(dat);
								ChunkInfo.Length--;
								}
								break;
						  case TilesetChunk_Lower_tile_passable://0x04,
								while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								ChipSet.vc_ch_Lower_tile_passable.push_back(Void);
							    }
								break;
						  case TilesetChunk_Upper_tile_passable://0x05,
								while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								ChipSet.vc_ch_Upper_tile_passable.push_back(Void);
								}
								break;
						  case TilesetChunk_Water_animation://0x0B,
							   ChipSet.Water_animation = ReadCompressedInteger(Stream);
							   break;
						  case TilesetChunk_Animation_speed://0x0C
							   ChipSet.Animation_speed = ReadCompressedInteger(Stream);
							   break;

						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
		  }while(ChunkInfo.ID!=0);
		  datareaded++;
		  vecChipset.push_back(ChipSet);
		  ChipSet.clear();
		  }
		  return(vecChipset);
}

std:: vector <stcEvent>  LDB_reader:: EventChunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std::vector <stcEvent> vecEvent;
		 stcEvent Event;
                 stEvent Event_parser;
		 Event.clear();
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		 do{
			ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				switch(ChunkInfo.ID)// tipo de la primera dimencion
				 {
					case Common_EventChunk_Name:
					     Event.strName = ReadString(Stream, ChunkInfo.Length);
					     break;
					case Common_EventChunk_NameActivation_condition://0x0B,
					     Event.intNameActivation_condition = ReadCompressedInteger(Stream);
					     break;
					case Common_EventChunk_NameActivate_on_switch://0x0C,
					     Event.blNameActivate_on_switch = ReadCompressedInteger(Stream);
					     break;
					case Common_EventChunk_NameSwitch_ID://0x0D,
					     Event.intNameSwitch_ID = ReadCompressedInteger(Stream);
					     break;
					case Common_EventChunk_NameScript_length://0x15,
					     Event.intNameScript_length = ReadCompressedInteger(Stream);
					     break;
					case Common_EventChunk_NameScript://0x16
				             Event.vcEvent_comand =  Event_parser.EventcommandChunk(Stream);
					     break;
					case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
			 }while(ChunkInfo.ID!=0);
		  datareaded++;
		 vecEvent.push_back(Event);
		 Event.clear();
		  }
		  return(vecEvent);
}

stcGlosary  LDB_reader::StringChunk(FILE * Stream)//movimientos de la pagina
{
	stcGlosary Glosary;
	do{
		ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
		if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
		ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
		switch(ChunkInfo.ID)// tipo de la primera dimencion
		{
			case  Enemy_encounter://0x01,
			      Glosary.Enemy_encounter = ReadString(Stream, ChunkInfo.Length);
			      break;
			case Headstart_attack://0x02,
			Glosary.Headstart_attack = ReadString(Stream, ChunkInfo.Length);
			     break;
			case Escape_success://0x03,
				Glosary.Escape_success = ReadString(Stream, ChunkInfo.Length);
			break;
			case Escape_failure://0x04,
				Glosary.Escape_failure = ReadString(Stream, ChunkInfo.Length);
				break;
			case Battle_victory://0x05,
				Glosary.Battle_victory = ReadString(Stream, ChunkInfo.Length);
				break;
			case Battle_defeat://0x06,
				Glosary.Battle_defeat = ReadString(Stream, ChunkInfo.Length);
				break;
			case Experience_received://0x07,
				Glosary.Experience_received = ReadString(Stream, ChunkInfo.Length);
				break;
			case Money_recieved_A://0x08,
				Glosary.Money_recieved_A = ReadString(Stream, ChunkInfo.Length);
				break;
			case Money_recieved_B://0x09,
				Glosary.Money_recieved_B = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Item_recieved://0x0A,
Glosary.Item_recieved = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Attack_message://0x0B,
Glosary.Attack_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Ally_critical_hit://0x0C,
Glosary.Ally_critical_hit = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Enemy_critical_hit://0x0D,
Glosary.Enemy_critical_hit = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Defend_message://0x0E,
Glosary.Defend_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Watch_message://0x0F,
Glosary.Watch_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Gathering_energy://0x10,
Glosary.Gathering_energy = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Sacrificial_attack://0x11,
Glosary.Sacrificial_attack = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Enemy_escape://0x12,
Glosary.Enemy_escape = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Enemy_transform://0x13,
Glosary.Enemy_transform = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Enemy_damaged://0x14,
Glosary.Enemy_damaged = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Enemy_undamaged://0x15,
Glosary.Enemy_undamaged = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Ally_damaged://0x16,
Glosary.Ally_damaged = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Ally_undamaged://0x17,
Glosary.Ally_undamaged = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Skill_failure_A://0x18,
								 Glosary.Skill_failure_A = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Skill_failure_B://0x19,
								 Glosary.Skill_failure_B = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Skill_failure_C://0x1A,
								 Glosary.Skill_failure_C = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Attack_dodged://0x1B,
								 Glosary.Attack_dodged = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Item_use://0x1C,
								 Glosary.Item_use = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Stat_recovery://0x1D,
								 Glosary.Stat_recovery = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Stat_increase://0x1E,
								 Glosary.Stat_increase = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Stat_decrease://0x1F,
								 Glosary.Stat_decrease = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Ally_lost_via_absorb://0x20,
								 Glosary.Ally_lost_via_absorb = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Enemy_lost_via_absorb://0x21,
								 Glosary.Enemy_lost_via_absorb = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Resistance_increase://0x22,
								 Glosary.Resistance_increase = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Resistance_decrease://0x23,
								 Glosary.Resistance_decrease = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Level_up_message://0x24,
								 Glosary.Level_up_message = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Skill_learned://0x25,
								 Glosary.Skill_learned = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Shop_greeting ://0x29,
								 Glosary.Shop_greeting = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Shop_regreeting ://0x2A,
								 Glosary.Shop_regreeting = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Buy_message ://0x2B,
								 Glosary.Buy_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Sell_message ://0x2C,
								 Glosary.Sell_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Leave_message ://0x2D,
								 Glosary.Leave_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Buying_message ://0x2E,
								 Glosary.Buying_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Quantity_to_buy ://0x2F,
								 Glosary.Quantity_to_buy = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Purchase_end ://0x30,
								 Glosary.Purchase_end = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Selling_message ://0x31,
								 Glosary.Selling_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Quantity_to_sell ://0x32,
								 Glosary.Quantity_to_sell = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Selling_end ://0x33,
								 Glosary.Selling_end = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Shop_greeting_2 ://0x36,
								 Glosary.Shop_greeting_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Shop_regreeting_2 ://0x37,
								 Glosary.Shop_regreeting_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Buy_message_2 ://0x38,
								 Glosary.Buy_message_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Sell_message_2 ://0x39,
								 Glosary.Sell_message_2 = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Leave_message_2 ://0x3A,
								 Glosary.Leave_message_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Buying_message_2 ://0x3B,
								 Glosary.Buying_message_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Quantity_to_buy_2 ://0x3C,
								 Glosary.Quantity_to_buy_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Purchase_end_2 ://0x3D,
								 Glosary.Purchase_end_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Selling_message_2 ://0x3E,
								 Glosary.Selling_message_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Quantity_to_sell_2 ://0x3F,
								 Glosary.Quantity_to_sell_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Selling_end_2 ://0x40,
								 Glosary.Selling_end_2 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Shop_greeting_3 ://0x43,
								 Glosary.Shop_greeting_3 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Shop_regreeting_3 ://0x44,
								 Glosary.Shop_regreeting_3 = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Buy_message_3 ://0x45,
								 Glosary.Buy_message_3 = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Sell_message_3 ://0x46,
								 Glosary.Sell_message_3 = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Leave_message_3 ://0x47,
								 Glosary.Leave_message_3 = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Buying_message_3 ://0x48,
								 Glosary.Buying_message_3 = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Quantity_to_buy_3 ://0x49,
								 Glosary.Quantity_to_buy_3 = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Purchase_end_3 ://0x4A,
								 Glosary.Purchase_end_3 = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Selling_message_3 ://0x4B,
								 Glosary.Selling_message_3 = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Quantity_to_sell_3 ://0x4C,
								 Glosary.Quantity_to_sell_3 = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Selling_end_3 ://0x4D,
								 Glosary.Selling_end_3 = ReadString(Stream, ChunkInfo.Length);
						   break;
							   case Inn_A_Greeting_A://0x50,
								 Glosary.Inn_A_Greeting_A = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Inn_A_Greeting_B://0x51,
								 Glosary.Inn_A_Greeting_B = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Inn_A_Greeting_C://0x52,
								 Glosary.Inn_A_Greeting_C = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Inn_A_Accept://0x53,
								 Glosary.Inn_A_Accept = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Inn_A_Cancel://0x54,
								 Glosary.Inn_A_Cancel = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Inn_B_Greeting_A://0x55,
								 Glosary.Inn_B_Greeting_A = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Inn_B_Greeting_B://0x56,
								 Glosary.Inn_B_Greeting_B = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Inn_B_Greeting_C://0x57,
								 Glosary.Inn_B_Greeting_C = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Inn_B_Accept://0x58,
								 Glosary.Inn_B_Accept = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Inn_B_Cancel://0x59,
								 Glosary.Inn_B_Cancel = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Loose_items://0x5C,
								 Glosary.Loose_items = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Equipped_items://0x5D,
								 Glosary.Equipped_items = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Monetary_Unit://0x5F,
								 Glosary.Monetary_Unit = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Combat_Command://0x65,
								 Glosary.Combat_Command = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Combat_Auto://0x66,
								 Glosary.Combat_Auto = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Combat_Run://0x67,
								 Glosary.Combat_Run = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Command_Attack://0x68,
								 Glosary.Command_Attack = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Command_Defend://0x69,
								 Glosary.Command_Defend = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Command_Item://0x6A,
								 Glosary.Command_Item = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Command_Skill://0x6B,
								 Glosary.Command_Skill = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Menu_Equipment://0x6C,
								 Glosary.Menu_Equipment = ReadString(Stream, ChunkInfo.Length);
							break;
							   case Menu_Save://0x6E,
								 Glosary.Menu_Save = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Menu_Quit://0x70,
								 Glosary.Menu_Quit = ReadString(Stream, ChunkInfo.Length);

							break;
							   case New_Game://0x72,
								 Glosary.New_Game = ReadString(Stream, ChunkInfo.Length);
						   break;
							   case Load_Game://0x73,
								 Glosary.Load_Game = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Exit_to_Windows://0x75,
								 Glosary.Exit_to_Windows = ReadString(Stream, ChunkInfo.Length);
						        break;
							   case Level://0x7B,
								 Glosary.Level = ReadString(Stream, ChunkInfo.Length);
							     break;
							   case Health://0x7C,
								 Glosary.Health = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Mana://0x7D,
								 Glosary.Mana = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Normal_status://0x7E,
								 Glosary.Normal_status = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case Experience ://0x7F,//(short)
								 Glosary.Experience = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Level_short ://0x80,//(short)
								 Glosary.Level_short = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Health_short ://0x81,//(short)
								 Glosary.Health_short = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Mana_short ://0x82,//(short)
								 Glosary.Mana_short = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Mana_cost://0x83,
								 Glosary.Mana_cost = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Attack://0x84,
								 Glosary.Attack = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Defense://0x85,
								 Glosary.Defense = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Mind://0x86,
								 Glosary.Mind = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Agility://0x87,
								 Glosary.Agility = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Weapon://0x88,
								 Glosary.Weapon = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Shield://0x89,
								 Glosary.Shield = ReadString(Stream, ChunkInfo.Length);
						   break;
							   case Armor://0x8A,
								 Glosary.Armor = ReadString(Stream, ChunkInfo.Length);
						      break;
							   case Helmet://0x8B,
								 Glosary.Helmet = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Accessory://0x8C,
								 Glosary.Accessory = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Save_game_message://0x92,
								 Glosary.Save_game_message = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case Load_game_message://0x93,
								 Glosary.Load_game_message = ReadString(Stream, ChunkInfo.Length);
							  break;
							   case Exit_game_message://0x94,
								 Glosary.Exit_game_message = ReadString(Stream, ChunkInfo.Length);
							 break;
							   case File_name://0x97,
								 Glosary.File_name = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case General_Yes://0x98,
								 Glosary.General_Yes = ReadString(Stream, ChunkInfo.Length);
							   break;
							   case General_No://0x99
								 Glosary.General_No = ReadString(Stream, ChunkInfo.Length);
							    break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
									   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
				 } while(ChunkInfo.ID!=0);
return(Glosary);
}




stcMusic_Background   LDB_reader::musicChunk(FILE * Stream)
{
stcMusic_Background  stcMusic;
  stcMusic.clear();
do
 {
            ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if(ChunkInfo.ID!=0)
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

            switch(ChunkInfo.ID)// segun el tipo
            {
               case 0x01:
                     stcMusic.Name_of_Music_Background     = ReadString(Stream, ChunkInfo.Length);
                    break;
               case 0x02:
                       stcMusic.Fade_in_time     = ReadCompressedInteger(Stream);
                    break;
               case 0x03:
                     stcMusic.Volume     = ReadCompressedInteger(Stream);
                    break;
               case 0x04:
                     stcMusic.Tempo     = ReadCompressedInteger(Stream);
                    break;
               case 0x05:
                     stcMusic.Balance     = ReadCompressedInteger(Stream);
                    break;
               case 0x00:
                    break;
               default:
                    while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                    break;
            }
 }while (ChunkInfo.ID!=0);
   ChunkInfo.ID=1;
 return(stcMusic);
}
std::vector <stcBattle_test>   LDB_reader::Batletest(FILE * Stream)
{
int id,datatoread=0,datareaded=0;
std::vector <stcBattle_test> vc_Battle_test;
stcBattle_test Battle_test;
Battle_test.clear();
datatoread=ReadCompressedInteger(Stream);//numero de datos
 while(datatoread>datareaded) // si no hay mas en el array
 {
   id= ReadCompressedInteger(Stream);//lectura de id 1 de array
   //printf("\n indice 1 %d",id);
do
 {
            ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            if(ChunkInfo.ID!=0)
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
            //printf("\n indice 2 %d",ChunkInfo.ID );
            switch(ChunkInfo.ID)// segun el tipo
            {

             case  Battle_test_Chunks_Perso_ID:
                   Battle_test.Hero_ID = ReadCompressedInteger(Stream);
                   break;
             case Battle_test_Chunks_Level:
                     Battle_test.Level = ReadCompressedInteger(Stream);
                     break;
               case Battle_test_Chunks_Weapon:
                      if(ChunkInfo.Length==4)
                    {
                    fread(&Void, sizeof(char), 1, Stream);
                    fread(&Void, sizeof(char), 1, Stream);
                    }
                      Battle_test.Weapon = ReadCompressedInteger(Stream);
                    break;
               case Battle_test_Chunks_Shield:
                    if(ChunkInfo.Length==4)
                    {
                    fread(&Void, sizeof(char), 1, Stream);
                    fread(&Void, sizeof(char), 1, Stream);
                    }
                      Battle_test.Shield = ReadCompressedInteger(Stream);
                    break;
               case Battle_test_Chunks_Armor:
                     if(ChunkInfo.Length==4)
                    {
                    fread(&Void, sizeof(char), 1, Stream);
                    fread(&Void, sizeof(char), 1, Stream);
                    }
                      Battle_test.Armor = ReadCompressedInteger(Stream);
                     break;
               case Battle_test_Chunks_Helmet:
                      if(ChunkInfo.Length==4)
                    {
                    fread(&Void, sizeof(char), 1, Stream);
                    fread(&Void, sizeof(char), 1, Stream);
                    }
                      Battle_test.Helmet = ReadCompressedInteger(Stream);
                     break;
              case Battle_test_Chunks_Accessory:
                    Battle_test.Accessory = ReadCompressedInteger(Stream);
                     break;
               case 0x00:
                    break;
               default:
                    while(ChunkInfo.Length--)
                    fread(&Void, sizeof(char), 1, Stream);
                    break;
            }
 }while (ChunkInfo.ID!=0);
 datareaded++;
vc_Battle_test.push_back(Battle_test);
Battle_test.clear();
}
ChunkInfo.ID=1;return(vc_Battle_test);
}

stcSystem  LDB_reader::SystemChunk(FILE * Stream)//movimientos de la pagina
{		 short dat;
		 stcSystem System;
			  do{
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				switch(ChunkInfo.ID)// tipo de la primera dimencion
				{
				case LDB_ID://0x0A,
				     System.intLDB_ID = ReadCompressedInteger(Stream);
				     break;
				case Skiff_graphic://0x0B,
				     System.Skiff_graphic = ReadString(Stream, ChunkInfo.Length);
				     break;
				case Boat_graphic://0x0C,
				     System.Boat_graphic = ReadString(Stream, ChunkInfo.Length);
				     break;
				case Airship_graphic://0x0D,
				     System.Airship_graphic = ReadString(Stream, ChunkInfo.Length);
				     break;
				case Skiff_index://0x0E,
				     System.Skiff_index = ReadCompressedInteger(Stream);
				     break;
				case Boat_index://0x0F,
				     System.Boat_index = ReadCompressedInteger(Stream);
				     break;
				case Airship_index://0x10,
				     System.Airship_index = ReadCompressedInteger(Stream);
				     break;
				case Title_graphic://0x11,
				     System.Title_graphic = ReadString(Stream, ChunkInfo.Length);
				     break;
				case Game_Over_graphic://0x12,
				     System.Game_Over_graphic = ReadString(Stream, ChunkInfo.Length);
				     break;
				case System_graphic://0x13,
				     System.System_graphic = ReadString(Stream, ChunkInfo.Length);
				     break;
				case System_graphic_2://0x14,
				     System.System_graphic_2 = ReadString(Stream, ChunkInfo.Length);
				     break;
				case Heroes_in_starting://0x15,
				     System.Heroes_in_starting = ReadCompressedInteger(Stream);
								 break;
				case Starting_party://0x16,
				     while(ChunkInfo.Length--){
					fread(&dat, sizeof(short), 1, Stream);
					System.vc_sh_Starting_party.push_back(dat);
					ChunkInfo.Length--;
					}
				     break;
				case Num_Comadns_order://0x1A,
				     System.intNum_Comadns_order = ReadCompressedInteger(Stream);
				     break;
				case Comadns_order://0x1B,
				     while(ChunkInfo.Length--){
					fread(&dat, sizeof(short), 1, Stream);
					System.vc_sh_Comadns_order.push_back(dat);
					ChunkInfo.Length--;
					}
				     break;
				case Title_music://0x1F,
					System.Title_music=musicChunk(Stream);//0x1F,
								 break;
				case Battle_music://0x20,
					System.Battle_music=musicChunk(Stream);//0x20,
								 break;
				case Battle_end_music://0x21,
					System.Battle_end_music=musicChunk(Stream);//0x21,
								 break;
				case Inn_music://0x21,
					System.Inn_music=musicChunk(Stream);//0x22,
								 break;
				case Skiff_music://0x21,
					System.Skiff_music=musicChunk(Stream);//0x23,
								 break;
				case Boat_music://0x21,
					System.Boat_music=musicChunk(Stream);//0x24,
								 break;
				case Airship_music://0x21,
					System.Airship_music=musicChunk(Stream);//0x25,
								 break;
				case Game_Over_music://0x21,
					System.Game_Over_music=musicChunk(Stream);//0x26,
								 break;
				case Cursor_SFX://0x21,
					System.Cursor_SFX=soundChunk(Stream);//0x29,
								 break;
				case Accept_SFX://0x21,
					System.Accept_SFX=soundChunk(Stream);//0x2A,
								 break;
				case Cancel_SFX://0x21,
					System.Cancel_SFX=soundChunk(Stream);//0x2B,
								 break;
				case Illegal_SFX://0x21,
					System.Illegal_SFX=soundChunk(Stream);//0x2C,
								 break;
				case Battle_SFX://0x21,
					System.Battle_SFX=soundChunk(Stream);//0x2D,
								 break;
				case Escape_SFX://0x21,
					System.Escape_SFX=soundChunk(Stream);//0x2E,
								 break;
				case Enemy_attack_SFX://0x21,
					System.Enemy_attack_SFX=soundChunk(Stream);//0x2F,
								 break;
				case Enemy_damaged_SFX://0x21,
					System.Enemy_damaged_SFX=soundChunk(Stream);//0x30,
								 break;
				case Ally_damaged_SFX://0x21,
					System.Ally_damaged_SFX=soundChunk(Stream);//0x31,
								 break;
				case Evasion_SFX://0x21,
					System.Evasion_SFX=soundChunk(Stream);//0x32,
								 break;
				case Enemy_dead_SFX://0x21,
					System.Enemy_dead_SFX=soundChunk(Stream);//0x33,
								 break;
				case Item_use_SFX://0x21,
					System.Item_use_SFX=soundChunk(Stream);//0x34,
								 break;
				case Map_exit_transition://0x3D,
				     System.Map_exit_transition = ReadCompressedInteger(Stream);
				     break;
				case Map_enter_transition://0x3E,
				     System.Map_enter_transition = ReadCompressedInteger(Stream);
				     break;
				case Battle_start_fadeout://0x3F,
					System.Battle_start_fadeout = ReadCompressedInteger(Stream);
								 break;
				case Battle_start_fadein://0x40,
					System.Battle_start_fadein = ReadCompressedInteger(Stream);
								 break;
				case Battle_end_fadeout://0x41,
					System.Battle_end_fadeout = ReadCompressedInteger(Stream);
								 break;
				case Battle_end_fadein://0x42,
					System.Battle_end_fadein = ReadCompressedInteger(Stream);
								 break;
				case Message_background://0x47,
					System.Message_background = ReadCompressedInteger(Stream);
								 break;
				case Font_id://0x48,
					System.Font = ReadCompressedInteger(Stream);
								 break;
				case Selected_condition://0x51,
					System.Selected_condition = ReadCompressedInteger(Stream);
					break;
				case Selected_hero://0x52,
					System.Selected_hero = ReadCompressedInteger(Stream);
								 break;
				case Battle_test_BG://0x54,
				     System.Battle_test_BG = ReadString(Stream, ChunkInfo.Length);
			              break;
				case Battle_test_data://0x55
				     System.vc_Battle_test=Batletest(Stream);
				     break;
				case Times_saved://0x41,
				     System.Times_saved = ReadCompressedInteger(Stream);
				     break;
				case Show_frame://0x42,
				     System.Show_frame = ReadCompressedInteger(Stream);
				     break;
				case In_battle_anim://0x47,
				     System.In_battle_anim = ReadCompressedInteger(Stream);
				     break;
				case CHUNK_LDB_END_OF_BLOCK:
							   break;
				default:
			        	while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
				}
			  }while(ChunkInfo.ID!=0);
	   return(System);
}
std:: vector <std::string>  LDB_reader::Switch_VariableChunk(FILE * Stream)//simplemente un vector de string
{
		 int id,datatoread=0,datareaded=0;
		 std::string name;
		 std:: vector <std::string> names;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			 do {
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

				if((id>datareaded)&&(ChunkInfo.ID==0))
				{names.push_back("");
					datareaded++;
					}

						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case 0x01:// nombres de los swiches
								 name = ReadString(Stream, ChunkInfo.Length);
								 names.push_back(name);
								 datareaded++;
								break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
				} while(ChunkInfo.ID!=0);

		  }
		 return(names);
}

std:: vector <stcCombatcommand>  LDB_reader:: Comand_Chunk2(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 stcCombatcommand Combatcommand;
		 Combatcommand.clear();
		 std:: vector <stcCombatcommand> vc_Combatcommand;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
		//printf("\n primer id Comand_Chunk2 %d ",id);
		 do {
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				//printf("\n segundo id Comand_Chunk2  %d ",ChunkInfo.ID);
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
				 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case 0x01:// nombres de los comandos
							Combatcommand.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						  case 0x02:// id de posicion
							Combatcommand.intUsage	= ReadCompressedInteger(Stream);
								break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
				} while(ChunkInfo.ID!=0);
                 vc_Combatcommand.push_back(Combatcommand);
		 Combatcommand.clear();
		  datareaded++;
		  }
		   ChunkInfo.ID	 = 1;//no afectar el otro ciclo
return(vc_Combatcommand);
}
stcCombatcommands  LDB_reader:: Comand_Chunk(FILE * Stream)//todo leido
{
     	 stcCombatcommands Combatcommands;
		Combatcommands.clear();
	 	do {

				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				//printf("\n primer id %d ",ChunkInfo.ID);
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
			      ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
 						case 0x02://entero
							  while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								}
							break;
						case 0x07://entero
							  while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								}
							break;
						case 0x09://entero
							  while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								}
							break;
						case 0x0A:
						Combatcommands.vc_Combatcommand=Comand_Chunk2(Stream);
								break;
						  case 0x14://entero
							  while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								}
								break;
						   case 0x18://entero
							  while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								}
								break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
				} while(ChunkInfo.ID!=0);
return(Combatcommands);
}



std:: vector <stcProfetion>   LDB_reader:: Profession_Chunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0,levels=0,comands=0;
		short dat;
		 std::vector <stcProfetion> vecProfetion;
		 stcProfetion Profetion;
		 Profetion.clear();
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			 do {
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				//printf("\n ID %d",  ChunkInfo.ID);
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
				          	  case Profetion_Name://0x01,
								 Profetion.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						  case  Profetion_TwoWeapon://0x15,
								Profetion.TwoWeapon = ReadCompressedInteger(Stream);
                                break;
						  case  Profetion_fixed_equipment ://0x16,
								Profetion.fixed_equipment = ReadCompressedInteger(Stream);
								break;
						  case  Profetion_AI_forced_action ://0x17,
								Profetion.AI_forced_action = ReadCompressedInteger(Stream);
								break;
						  case  Profetion_strong_defense ://0x18,
								Profetion.strong_defense = ReadCompressedInteger(Stream);
								break;
 						  case  Profetion_Each_level ://0x1F,
						levels=ChunkInfo.Length/6;
						 while(levels--)
						 {fread(&dat, sizeof(short), 1, Stream);
						  Profetion.vc_sh_Hp.push_back(dat);
						 levels--;
						 }
						 levels=ChunkInfo.Length/6;
						 while(levels--)
						 {fread(&dat, sizeof(short), 1, Stream);
						  Profetion.vc_sh_Mp.push_back(dat);
						 levels--;
						 }
						  levels=ChunkInfo.Length/6;
						 while(levels--)
						 {fread(&dat, sizeof(short), 1, Stream);
						  Profetion.vc_sh_Attack.push_back(dat);
						 levels--;
						 }
						  levels=ChunkInfo.Length/6;
						 while(levels--)
						 {fread(&dat, sizeof(short), 1, Stream);
						  Profetion.vc_sh_Defense.push_back(dat);
					   levels--;
						 }
						  levels=ChunkInfo.Length/6;
						 while(levels--)
						 {fread(&dat, sizeof(short), 1, Stream);
						  Profetion.vc_sh_Mind.push_back(dat);
						 levels--;
						 }
						 levels=ChunkInfo.Length/6;
						 while(levels--)
						 {fread(&dat, sizeof(short), 1, Stream);
						  Profetion.vc_sh_Agility.push_back(dat);
						 levels--;
						 }
								break;
 						  case  Profetion_Experience_curve_basic_values ://0x29,
								Profetion.Experience_curve_basic_values = ReadCompressedInteger(Stream);
								break;
						  case  Profetion_Experience_curve_increase_degree ://0x2A,
								Profetion.Experience_curve_increase_degree = ReadCompressedInteger(Stream);
								break;
 						  case  Profetion_Experience_curve_correction_value://0x2B,
								Profetion.Experience_curve_correction_value = ReadCompressedInteger(Stream);
								break;
 						  case  Profetion_Animated_battle	://0x3E,
								Profetion.Animated_battle = ReadCompressedInteger(Stream);
								break;
 						  case  Profetion_Special_skills_level://0x3F,
   							  	Profetion.skills=heroskillChunk(Stream);
							   break;
 						  case  Profetion_Effectiveness_state_number ://0x47,
								Profetion.Effectiveness_state_number = ReadCompressedInteger(Stream);
								break;
						  case Profetion_Effectiveness_state_data:
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								Profetion.vc_ch_Condeffects.push_back(Void);
								}
								break;

                          case  Profetion_Effectiveness_Attribute_number ://0x49,
								Profetion.Effectiveness_Attribute_number = ReadCompressedInteger(Stream);
								break;
						  case Profetion_Effectiveness_Attribute_data://0x4A,
							   while(ChunkInfo.Length--)
								{fread(&Void, sizeof(char), 1, Stream);
								Profetion.vc_ch_Attribeffects.push_back(Void);
								}
								break;
						  case Profetion_Combat_Command://0x50
							while(ChunkInfo.Length--) //4 chars
							{fread(&comands, sizeof(int), 1, Stream);
							Profetion.vc_int_Combat_Command.push_back(comands);
							ChunkInfo.Length-=3;}
							break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
				} while(ChunkInfo.ID!=0);
		  datareaded++;
		 vecProfetion.push_back(Profetion);
		 Profetion.clear();
		  }
		  return(vecProfetion);
}
std:: vector <stcFight_anim>  LDB_reader:: Fightanim_Chunk2(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std::vector <stcFight_anim> vecFight_anim;
		 stcFight_anim Fight_anim;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			 do {
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case Name:// nombres de los swiches
								 Fight_anim.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						  case Filename:// nombres de los swiches
							Fight_anim.strFilename = ReadString(Stream, ChunkInfo.Length);
							break;
						  case 	Position://=0x03,
							Fight_anim.intPosition=ReadCompressedInteger(Stream);
							break;
						  case 	Extended_expanded://=0x04,
							Fight_anim.blExtended_expanded=ReadCompressedInteger(Stream);
							break;
						  case 	ID_Expansion_animated_combat_ID://=0x05
				          	Fight_anim.intID_Expansion_animated_combat_ID=ReadCompressedInteger(Stream);
							break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
						  default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
				} while(ChunkInfo.ID!=0);
		  datareaded++;
		 vecFight_anim.push_back(Fight_anim);
		 Fight_anim.clear();
		  } ChunkInfo.ID=1;
		  return(vecFight_anim);
}
std:: vector <stcBattle_comand>  LDB_reader:: Fightanim_Chunk(FILE * Stream)
{
		 int id,datatoread=0,datareaded=0;
		 std::vector <stcBattle_comand> vecFight_anim;
		 stcBattle_comand Fight_anim;
		 datatoread=ReadCompressedInteger(Stream);//numero de datos
		 while(datatoread>datareaded) // si no hay mas en el array
		 {
		 id= ReadCompressedInteger(Stream);//lectura de id 1 de array
			 do {
				ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
				//printf("%d ",ChunkInfo.ID);
				if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
				ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
						 switch(ChunkInfo.ID)// tipo de la primera dimencion
						 {
						  case Animated_battle_Name:// nombres de los swiches
								 Fight_anim.strName = ReadString(Stream, ChunkInfo.Length);
								break;
						 case Animated_battle_Motion_attack:
							Fight_anim.intMotion_attack=ReadCompressedInteger(Stream);
							break;

						case Animated_battle_Combat_Anime:
							Fightanim_Chunk2(Stream);
							break;
						case Animated_battle_Anime_combat_weapons:
							Fightanim_Chunk2(Stream);
							break;
						  case CHUNK_LDB_END_OF_BLOCK:
							   break;
							   default:
							   // saltate un pedazo del tamaño de la longitud
							   while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
							   break;
						  }
				} while(ChunkInfo.ID!=0);
		  datareaded++;
		 vecFight_anim.push_back(Fight_anim);
		 Fight_anim.clear();
		  }
		  return(vecFight_anim);
}


void  LDB_reader::GetNextChunk(FILE * Stream, LDB_data * data)
{

		unsigned char Void;
		tChunk ChunkInfo; // informacion del pedazo leido
		// Loop while we haven't reached the end of the file
		while(!feof(Stream) && (!stop))
		{
			ChunkInfo.ID	 = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
			ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
		 if(ChunkInfo.Length>0)
			switch(ChunkInfo.ID)// segun el tipo
			{

			 case CHUNK_Hero_data:
					data->heros= heroChunk(Stream);
				 break;
			 case CHUNK_Skill:
					data->skill= skillChunk(Stream);
					break;
			 case CHUNK_Item_data:
					data->items=itemChunk(Stream);
					break;
			case CHUNK_Monster:
					data->mosters= mosterChunk(Stream);
					break;
			case CHUNK_MonsterP:
					data->mosterpartys=mosterpartyChunk(Stream);
					break;
			case CHUNK_Terrain:
					data->Terrains=TerrainChunk(Stream);
					break;
			case CHUNK_Attribute:
                    data->Attributes=AttributeChunk(Stream);
					break;
			case CHUNK_States:
                    data->States=StatesChunk(Stream);
					break;
			case CHUNK_Animation:
                    data->Animations=AnimationChunk(Stream);
					break;
			case CHUNK_Tileset:
                    data->Tilesets=TilesetChunk(Stream);
					break;
			case CHUNK_String:
                    data->Glosary=StringChunk(Stream);
					break;
			case CHUNK_System:
                    data->System_dat=SystemChunk(Stream);
					break;
			case CHUNK_Event:
                    data->Event=EventChunk(Stream);
					break;
			case CHUNK_Switch:
                    data->Switch_Names= Switch_VariableChunk(Stream);
					break;
			case CHUNK_Variable:
                    data->Variable_Names =Switch_VariableChunk(Stream);
					break;
			case CHUNK_Event1://no existe
				while(ChunkInfo.Length--)
				{
				fread(&Void, sizeof(char), 1, Stream);
				printf("%d",Void);
				}

					break;
			case CHUNK_Event2://no existe
				while(ChunkInfo.Length--)
				{
				fread(&Void, sizeof(char), 1, Stream);
				printf("%d",Void);
				}
					break;
			case CHUNK_Event3://no existe
				while(ChunkInfo.Length--)
				{
				fread(&Void, sizeof(char), 1, Stream);
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

				while(ChunkInfo.Length--)
				{
				fread(&Void, sizeof(char), 1, Stream);
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
                while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                break;
			}

		}
	}

void  LDB_reader::ShowInformation(LDB_data * data)
{ // muestra de informacion del mapa
    int j,i;
    j=data->heros.size();
    for (i=0;i<j ;i++)
        data->heros[i].show();
    j=data->Professions.size();
    for (i=0;i<j ;i++)
        data->Professions[i].show();
    data->Glosary.show();
    data->System_dat.show();
    data->Combatcommands.show();
}
