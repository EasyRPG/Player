#include "ldb.h"
#include <iostream>
void stcSound_effect::clear(){
Name_of_Sound_effect="";
 Volume=100;
 Tempo=100;
 Balance=50;
}

void stcSound_effect::show(){
printf(" \n Name_of_Sound_effect %s",Name_of_Sound_effect.c_str());
printf(" \n Volume %d",Volume);
printf(" \n Tempo %d",Tempo);
printf(" \n Balance %d",Balance);
}

void stcMusic_Background::clear(){
Name_of_Music_Background="";
Fade_in_time=100;
 Volume=100;
 Tempo=100;
 Balance=50;
}

void stcMusic_Background::show(){
printf(" \n Name_of_Sound_effect %s",Name_of_Music_Background.c_str());
printf(" \n Volume %d",Fade_in_time);
printf(" \n Volume %d",Volume);
printf(" \n Tempo %d",Tempo);
printf(" \n Balance %d",Balance);
}

void Magicblock::clear()
{
 Level=0;//=0x01,
 Spell_ID=0;//=0x02
}
void Magicblock::show()
{
 printf(" \n Level %d",Level);//=0x01,
 printf(" \n Spell_ID %d",Spell_ID);//=0x02
}
void stcHero::clear()
{
strName="";//=0x01,
strClass="";//=0x02,
strGraphicfile="";//=0x03,
intGraphicindex=0;//=0x04,
intTransparent=0;//=0x05,
intStartlevel=1;//=0x07,
intMaxlevel=50;//=0x08,
intCrithit=1;//=0x09,
intHitchance=30;//=0x0A,
strFacegraphic="";//=0x0F,
intFaceindex=0;//=0x10,
blDualwield=0;//=0x15,
blFixedequipment=0;//=0x16,
blAI=0;//=0x17,
blHighdefense=0;//=0x18,
//Statisticscurves=0x1F,arraglode16bitescon5datos
vc_sh_Hp.clear();
vc_sh_Mp.clear();
vc_sh_Attack.clear();
vc_sh_Defense.clear();
vc_sh_Mind.clear();
vc_sh_Agility.clear();
intEXPBaseline=30;//=0x29,
intEXPAdditional=30;//=0x2A,
intEXPCorrection=30;//=0x2B,
intprofesion=0;//0x39--2003
Animatedbattle=0;//0x3E--2003
//Startequip
sh_Weapon=0;//
sh_Shield=0;//
sh_Armor=0;//
sh_Head=0;//
sh_Accessory=0;//
skills.clear();//Skills=0x3F,
blRenameMagic=0;//=0x42,
strMagicname="";//=0x43,
intCondlength=0;//=0x47,
vc_ch_Condeffects.clear();//arraydeunbyte
intAttriblength=0;//=0x49,
vc_ch_Attribeffects.clear();//array de un byte
vc_int_Combat_Command.clear();//Combat Comand // =0x50 //4 byte one byte dimension array of 6
}

void stcHero::show()
{
unsigned int i;
printf(" \n Name %s",strName.c_str());
//std::cout << "Name " << strName << std::endl;
printf(" \n Class %s",strClass.c_str());
printf(" \n Graphicfile %s",strGraphicfile.c_str());
printf(" \n Graphicindex %d",intGraphicindex);
printf(" \n Transparent %d",intTransparent);
printf(" \n Startlevel %d",intStartlevel);
printf(" \n Maxlevel %d",intMaxlevel);
printf(" \n Crithit %d",intCrithit);
printf(" \n Hitchance %d",intHitchance);
printf(" \n Facegraphic %s",strFacegraphic.c_str());
printf(" \n intFaceindex %d",intFaceindex);
printf(" \n blDualwield %d",intHitchance);
printf(" \n blFixedequipment %d",blFixedequipment);
printf(" \n blAI %d",blAI);
printf(" \n blHighdefense %d",blHighdefense);
//Statisticscurves=0x1F,arraglode16bitescon5datos
for(i=0;i<(vc_sh_Hp.size());i++)
printf(" \n hero hp %d",vc_sh_Hp[i]);
//vc_sh_Mp.clear();
//vc_sh_Attack.clear();
//vc_sh_Defense.clear();
//vc_sh_Mind.clear();
//vc_sh_Agility.clear();
printf(" \n intEXPBaseline %d",intEXPBaseline);
printf(" \n intEXPAdditional %d",intEXPAdditional);
printf(" \n intEXPCorrection %d",intEXPCorrection);
printf(" \n intprofesion %d",intprofesion);
printf(" \n Animatedbattle %d",Animatedbattle);
//Startequip
printf(" \n sh_Weapon %d",sh_Weapon);
printf(" \n sh_Shield %d",sh_Shield);
printf(" \n sh_Armor %d",sh_Armor);
printf(" \n sh_Head %d",sh_Head);
printf(" \n sh_Accessory %d",sh_Accessory);
//skills.clear();//Skills=0x3F,
printf(" \n blRenameMagic %d",blRenameMagic);
strMagicname="";//=0x43,
printf(" \n intCondlength %d",intCondlength);
//vc_ch_Condeffects.clear();//arraydeunbyte
printf(" \n intAttriblength %d",intAttriblength);
//vc_ch_Attribeffects.clear();//array de un byte
//vc_int_Combat_Command.clear();//Combat Comand // =0x50 //4 byte one byte dimension array of 6
}


void stcSkill::clear()
{
strName="";//0x01,
strDescription="";//0x02,
strtext="";//0x03,
strtextline="";//0x04,
intFailure=0;//0x07,
intSpelltype=0;//0x08,
intMpconType=0;//0x09    //2003
intConsumtion=0;//0x0A  //2003
intCost=0;//0x0B,
intRange=0;//0x0C,
intSwitch=0;//0x0D,
intBattleanimation=0;//0x0E,
blFieldusage=0;//0x12,
blCombatusage=0;//0x13,
blChangeofstatus=0;//0x14 //-2003
intStrengtheffect=0;//0x15,
intMindeffect=0;//0x16,
intVariance=0;//0x17,
intBasevalue=0;//0x18,
intSuccessrate=0;//0x19,
blAffectHP=0;//0x1F,
blAffectMP=0;//0x20,
blAffectAttack=0;//0x21,
blAffectDefense=0;//0x22,
blAffectMind=0;//0x23,
blAffectAgility=0;//0x24,
blAbsorbdamage=0;//0x25,
blIgnoredefense=0;//0x26,
intConditionslength=0;//0x29,
vc_ch_Condeffects.clear();//0x2A,
intAttributeslength=0;//0x2B,
vc_ch_Attribeffects.clear(); //0x2C,

blAffectresistance=0;//0x2D
intwhenusinganimation=0; //0x31 --2003
//CBA data  //0x32
}

void stcSkill::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n strDescription %s",strDescription.c_str());
printf(" \n strtext %s",strtext.c_str());
printf(" \n strtextline %s",strtextline.c_str());
printf(" \n intFailure %d",intFailure);
printf(" \n intSpelltype %d",intSpelltype);
printf(" \n intMpconType %d",intMpconType);
printf(" \n intConsumtion %d",intConsumtion);
printf(" \n intCost %d",intCost);
printf(" \n intRange %d",intRange);
printf(" \n intSwitch %d",intSwitch);
printf(" \n intBattleanimation %d",intBattleanimation);
printf(" \n blFieldusage %d",blFieldusage);
printf(" \n blCombatusage %d",blCombatusage);
printf(" \n blChangeofstatus %d",blChangeofstatus);
printf(" \n intStrengtheffect %d",intStrengtheffect);
printf(" \n intMindeffect %d",intMindeffect);
printf(" \n intVariance %d",intVariance);
printf(" \n intBasevalue %d",intBasevalue);
printf(" \n intSuccessrate %d",intSuccessrate);
printf(" \n blAffectHP %d",blAffectHP);
printf(" \n blAffectMP %d",blAffectMP);
printf(" \n blAffectAttack %d",blAffectAttack);
printf(" \n blAffectDefense %d",blAffectDefense);
printf(" \n blAffectMind %d",blAffectMind);
printf(" \n blAffectAgility %d",blAffectAgility);
printf(" \n blAbsorbdamage %d",blAbsorbdamage);
printf(" \n blIgnoredefense %d",blIgnoredefense);
printf(" \n intConditionslength %d",intConditionslength);
//vc_ch_Condeffects.clear();//0x2A,
printf(" \n intAttributeslength %d",intAttributeslength);
//vc_ch_Attribeffects.clear(); //0x2C,

printf(" \n blAffectresistance %d",blAffectresistance);
printf(" \n intwhenusinganimation %d",intwhenusinganimation);
//CBA data  //0x32
}

void stcItem::clear()
{
Name="";//0x01,
Description="";//0x02,
Type=0;//0x03,
Cost=0;//0x05,
Uses=0;//0x06,
Attack=0;//0x0B,
Defense=0;//0x0C,
Mind=0;//0x0D,
Speed=0;//0x0E,
Equip=0;//0x0F,
MPcost=0;//0x10,
Chancetohit=0;//0x11,
Criticalhit=0;//0x12,
Battleanimation=0;//0x14,
Preemptiveattack=0;//0x15,
Doubleattack=0;//0x16,
Attackallenemies=0;//0x17,
Ignoreevasion=0;//0x18,
Preventcriticalhits=0;//0x19,
Raiseevasion=0;//0x1A,
MPusecutinhalf=0;//0x1B,
Noterraindamage=0;//0x1C,
Is_equip=0;//0x1D//2003
Healsparty=0;//0x1F,
HPrecovery=0;//0x20,
HPrecoveryvalue=0;//0x21,
MPrecovery=0;//0x22,
MPrecoveryvalue=0;//0x23,
Useonfieldonly=0;//0x25,
Onlyondeadheros=0;//0x26,
MaxHPmodify=0;//0x29,
MaxMPmodify=0;//0x2A,
Attackmodify=0;//0x2B,
Defensemodify=0;//0x2C,
Mindmodify=0;//0x2D,
Speedmodify=0;//0x2E,
Usagemessage=0;//0x33,
Efectiveastool=0;//0x35//2003
Switchtoturnon=0;//0x37,
Useonfield=0;//0x39,
Useinbattle=0;//0x3A,
Heroeslength=0;//0x3D,
vc_ch_Heroescanuse.clear();
Conditionslength=0;//0x3F,
vc_ch_Condeffects.clear();
Attributeslength=0;//0x41,
vc_ch_Attribeffects.clear();
Chancetochange=0;//0x43
Whenuseanimation=0;//0x45
ItemtoSkill=0;//0x47
EquitoVoc=0;//0x48
}

void stcItem::show()
{
printf(" \n Name %s",Name.c_str());
printf(" \n Description %s",Description.c_str());
printf(" \n Type %d",Type);
printf(" \n Cost %d",Cost);
printf(" \n Uses %d",Uses);
printf(" \n Attack %d",Attack);
printf(" \n Defense %d",Defense);
printf(" \n Mind %d",Mind);
printf(" \n Speed %d",Speed);
printf(" \n Equip %d",Equip);
printf(" \n MPcost %d",MPcost);
printf(" \n Chancetohit %d",Chancetohit);
printf(" \n Criticalhit %d",Criticalhit);
printf(" \n Battleanimation %d",Battleanimation);
printf(" \n Preemptiveattack %d",Preemptiveattack);
printf(" \n Doubleattack %d",Doubleattack);
printf(" \n Attackallenemies %d",Attackallenemies);
printf(" \n Ignoreevasion %d",Ignoreevasion);
printf(" \n Preventcriticalhits %d",Preventcriticalhits);
printf(" \n Raiseevasion %d",Raiseevasion);
printf(" \n MPusecutinhalf %d",MPusecutinhalf);
printf(" \n Noterraindamage %d",Noterraindamage);
printf(" \n Is_equip %d",Is_equip);
printf(" \n Healsparty %d",Healsparty);
printf(" \n HPrecovery %d",HPrecovery);
printf(" \n HPrecoveryvalue %d",HPrecoveryvalue);
printf(" \n MPrecovery %d",MPrecovery);
printf(" \n MPrecoveryvalue %d",MPrecoveryvalue);
printf(" \n Useonfieldonly %d",Useonfieldonly);
printf(" \n Onlyondeadheros %d",Onlyondeadheros);
printf(" \n MaxHPmodify %d",MaxHPmodify);
printf(" \n MaxMPmodify %d",MaxMPmodify);
printf(" \n Attackmodify %d",Attackmodify);
printf(" \n Defensemodify %d",Defensemodify);
printf(" \n Mindmodify %d",Mindmodify);
printf(" \n Speedmodify %d",Speedmodify);
printf(" \n Usagemessage %d",Usagemessage);
printf(" \n Efectiveastool %d",Efectiveastool);
printf(" \n Switchtoturnon %d",Switchtoturnon);
printf(" \n Useonfield %d",Useonfield);
printf(" \n Useinbattle %d",Useinbattle);
printf(" \n Heroeslength %d",Heroeslength);
//vc_ch_Heroescanuse.clear();
printf(" \n Conditionslength %d",Conditionslength);
//vc_ch_Condeffects.clear();
printf(" \n Attributeslength %d",Attributeslength);
//vc_ch_Attribeffects.clear();
printf(" \n Chancetochange %d",Chancetochange);
printf(" \n Whenuseanimation %d",Whenuseanimation);
printf(" \n ItemtoSkill %d",ItemtoSkill);
printf(" \n EquitoVoc %d",EquitoVoc);
}

void stcEnemy_Action::clear()
{
  intAction=0;//=0x01,
  intAction_data=0;//	=0x02,
  intSkill_ID=0;//=0x03,
  intMonster_ID=0;//=0x04,
  intCondition=0;//	=0x05,
  intLower_limit=0;//=0x06,
  intUpper_limit=0;//	=0x07,
  intPriority=0;//=0x0D
}
void stcEnemy_Action::show()
{
printf(" \n intAction %d",intAction);
printf(" \n intAction_data %d",intAction_data);
printf(" \n intSkill_ID %d",intSkill_ID);
printf(" \n intMonster_ID %d",intMonster_ID);
printf(" \n intCondition %d",intCondition);
printf(" \n intLower_limit %d",intLower_limit);
printf(" \n intUpper_limit %d",intUpper_limit);
printf(" \n intPriority %d",intPriority);
}
void stcEnemy::clear()
{
   strName="";	// = 0x01,
   strGraphicfile="";	//= 0x02,
  intHuealteration=0;	//= 0x03,
  intMaxHP=0;	//= 0x04,
  intMaxMP=0;	//= 0x05,
  intAttack=0;	//= 0x06,
  intDefense=0;	//= 0x07,
  intMind=0;	//= 0x08,
  intSpeed=0;	//= 0x09,
  blTranslucentgraphic=0;	//= 0x0A,
  intExperience=0;	//= 0x0B,
  intGold=0;	//= 0x0C,
  intSpoilsitemID=0;	//= 0x0D,
  intSpoilschance=0;	//= 0x0E,
  blCanusecriticalhits=0;	//= 0x15,
  intCriticalhitchance=0;	//= 0x16,
  blUsuallymiss=0;	//= 0x1A,
  blAirborne=0;	//= 0x1C,
  intConditionslength=0;	//= 0x1F,
  vc_ch_Condeffects.clear(); //= 0x20, arreglo de un byte //una dimencion
  inyAttributeslength=0;//= 0x21,
  vc_ch_Attribeffects.clear();//= 0x22, arreglo de un byte //una dimencion
  //Actionslist= 0x2A
}

void stcEnemy::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n strGraphicfile %s",strGraphicfile.c_str());
printf(" \n intHuealteration %d",intHuealteration);
printf(" \n intMaxHP %d",intMaxHP);
printf(" \n intMaxMP %d",intMaxMP);
printf(" \n intAttack %d",intAttack);
printf(" \n intDefense %d",intDefense);
printf(" \n intMind %d",intMind);
printf(" \n intSpeed %d",intSpeed);
printf(" \n blTranslucentgraphic %d",blTranslucentgraphic);
printf(" \n intExperience %d",intExperience);
printf(" \n intGold %d",intGold);
printf(" \n intSpoilsitemID %d",intSpoilsitemID);
printf(" \n intSpoilschance %d",intSpoilschance);
printf(" \n blCanusecriticalhits %d",blCanusecriticalhits);
printf(" \n intCriticalhitchance %d",intCriticalhitchance);
printf(" \n blUsuallymiss %d",blUsuallymiss);
printf(" \n blAirborne %d",blAirborne);
printf(" \n intConditionslength %d",intConditionslength);
//  vc_ch_Condeffects.clear(); //= 0x20, arreglo de un byte //una dimencion
printf(" \n inyAttributeslength %d",inyAttributeslength);
//  vc_ch_Attribeffects.clear();//= 0x22, arreglo de un byte //una dimencion
  //Actionslist= 0x2A
}

void stcEnemy_group_data::clear()
{
	Enemy_ID=0;//=0x01,
	X_position=0;//=0x02,
	Y_position=0;//=0x03
}
void stcEnemy_group_data::show()
{
printf(" \n Enemy_ID %d",Enemy_ID);
printf(" \n X_position %d",X_position);
printf(" \n Y_position %d",Y_position);
}
void stcEnemy_group_condition::clear()
{
    Condition_flags=0;//=0x01,
    Switch_A=0;//=0x02,
    Turn_number_A=0;//=0x06,
	Lower_limit=0;//=0x0B,
	Upper_limit=0;//=0x0C,
}
void stcEnemy_group_condition::show()
{
printf(" \n Condition_flags %d",Condition_flags);
printf(" \n Switch_A %d",Switch_A);
printf(" \n Turn_number_A %d",Turn_number_A);
printf(" \n Lower_limit %d",Lower_limit);
printf(" \n Upper_limit %d",Upper_limit);
}
void stcEnemy_group_event_page::clear()
{
    conditions.clear();//	Page_conditions=0x02,
	Event_length=0;//=0x0B,
//	Event=0x0C// como en eventos
}

void stcEnemy_group::clear()
{
 strName="";// =0x01,
 Enemy_data.clear();//data=0x02,
 intTerrainlength=0;		//=0x04,
 vc_ch_Terraindata.clear();		//=0x05,
 vecPartyMonsterevent.clear();
}
void stcEnemy_group::show()
{
printf(" \n strName %s",strName.c_str());
// Enemy_data.clear();//data=0x02,
printf(" \n intTerrainlength %d",intTerrainlength);
//vc_ch_Terraindata.clear();		//=0x05,
// vecPartyMonsterevent.clear();
}


void stcTerrain::clear()
{
strName="";	//=0x01,
intDamageontravel=0;	//=0x02,
intEncounterate=0;	//=0x03,
strBattlebackground="";	//=0x04,
blSkiffmaypass=0;	//=0x05,
blBoatmaypass=0;	//=0x06,
blAirshipmaypass=0;	//=0x07,
blAirshipmayland=0;	//=0x09,
intHeroopacity=0;	//=0x0B
}
void stcTerrain::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n intDamageontravel %d",intDamageontravel);
printf(" \n intEncounterate %d",intEncounterate);
printf(" \n strBattlebackground %s",strBattlebackground.c_str());
printf(" \n blSkiffmaypass %d",blSkiffmaypass);
printf(" \n blBoatmaypass %d",blBoatmaypass);
printf(" \n blAirshipmaypass %d",blAirshipmaypass);
printf(" \n blAirshipmayland %d",blAirshipmayland);
printf(" \n intHeroopacity %d",intHeroopacity);
}
void stcAttribute::clear()
{
strName="";//=0x01,
intType=0;//=0x02,
intA_damage=0;//=0x0B,
intB_damage=0;//=0x0C,
intC_damage=0;//=0x0F,
intD_damage=0;//e=0x0F,
intE_damage=0;//=0x0F
}
void stcAttribute::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n intType %d",intType);
printf(" \n intA_damage %d",intA_damage);
printf(" \n intB_damage %d",intB_damage);
printf(" \n intC_damage %d",intC_damage);
printf(" \n intD_damage %d",intD_damage);
printf(" \n intE_damage %d",intE_damage);
}


void stcState::clear()
{
strName="";//=0x01,
intLength=0;//=0x02,
intColor=0;//=0x03,
intPriority=0;//=0x04,
intLimitation=0;//=0x05,
intA_chance=0;//=0x0B,
intB_chance=0;//=0x0C,
intC_chance=0;//=0x0D,
intD_chance=0;//=0x0E,
intE_chance=0;//=0x0F,
intTurnsforhealing=0;//=0x15,
intHealperturn=0;//=0x16,
intHealonshock=0;//=0x17,
intTypeofincrease=0;//0x1E-2003
blHalveAttack=0;//=0x1F,
blHalveDefense=0;//=0x20,
blHalveMind=0;//=0x21,
blHalveAgility=0;//=0x22,
intHitratechange=0;//=0x23,
blAvoidatack=0;//0x24--2003
blmagicreflection=0;//0x25--2003
blItemequip=0;//0x26--2003
intAnime=0;//0x27--2003
blPrevent_skill_use=0;//=0x29,
intMinimum_skill_level=0;//=0x2A,
blPreventmagicuse=0;//=0x2B,
intMinimummindlevel=0;//=0x2C,
intmpdecreasetype=0;//0x2D-2003
inthpdecreasetype=0;//0x2E-2003
strAllyenterstate="";//=0x33,
strEnemyentersstate="";//=0x34,
strAlreadyinstate="";//=0x35,
strAffectedbystate="";//=0x36,
strStatusrecovered="";//=0x37,
intHPloss=0;//=0x3D,
intHPlossvalue=0;//=0x3E,
intHPmaploss=0;//=0x3F,
intHPmapsteps=0;//=0x40,
intMPloss=0;//=0x41,
intMPlossvalue=0;//=0x42,
intMPmaploss=0;//=0x43,
intMPmapsteps=0;//=0x44
}

void stcState::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n intLength %d",intLength);
printf(" \n intColor %d",intColor);
printf(" \n intPriority %d",intPriority);
printf(" \n intLimitation %d",intLimitation);
printf(" \n intA_chance %d",intA_chance);
printf(" \n intB_chance %d",intB_chance);
printf(" \n intC_chance %d",intC_chance);
printf(" \n intD_chance %d",intD_chance);
printf(" \n intE_chance %d",intE_chance);
printf(" \n intTurnsforhealing %d",intTurnsforhealing);
printf(" \n intHealperturn %d",intHealperturn);
printf(" \n intHealonshock %d",intHealonshock);
printf(" \n intTypeofincrease %d",intTypeofincrease);
printf(" \n blHalveAttack %d",blHalveAttack);
printf(" \n blHalveDefense %d",blHalveDefense);
printf(" \n blHalveMind %d",blHalveMind);
printf(" \n blHalveAgility %d",blHalveAgility);
printf(" \n intHitratechange %d",intHitratechange);
printf(" \n blAvoidatack %d",blAvoidatack);
printf(" \n blmagicreflection %d",blmagicreflection);
printf(" \n blItemequip %d",blItemequip);
printf(" \n intAnime %d",intAnime);
printf(" \n blPrevent_skill_use %d",blPrevent_skill_use);
printf(" \n intMinimum_skill_level %d",intMinimum_skill_level);
printf(" \n blPreventmagicuse %d",blPreventmagicuse);
printf(" \n intMinimummindlevel %d",intMinimummindlevel);
printf(" \n intmpdecreasetype %d",intmpdecreasetype);
printf(" \n inthpdecreasetype %d",inthpdecreasetype);
printf(" \n strAllyenterstate %s",strAllyenterstate.c_str());
printf(" \n strEnemyentersstate %s",strEnemyentersstate.c_str());
printf(" \n strAlreadyinstate %s",strAlreadyinstate.c_str());
printf(" \n strAffectedbystate %s",strAffectedbystate.c_str());
printf(" \n strStatusrecovered %s",strStatusrecovered.c_str());
printf(" \n intHPloss %d",intHPloss);
printf(" \n intHPlossvalue %d",intHPlossvalue);
printf(" \n intHPmaploss %d",intHPmaploss);
printf(" \n intHPmapsteps %d",intHPmapsteps);
printf(" \n intMPloss %d",intMPloss);
printf(" \n intMPlossvalue %d",intMPlossvalue);
printf(" \n intMPmaploss %d",intMPmaploss);
printf(" \n intMPmapsteps %d",intMPmapsteps);
}


void stcAnimationTiming::clear()
{
		Frame=0;//=0x01,
		Sound_effect.clear();//=0x02,
		Flash_effect=0;//=0x03,
		Green_component=0;//=0x05,
		Blue_component=0;//=0x06,
		Flash_power=0;//=0x07
}

void stcAnimationTiming::show()
{
printf(" \n Frame %d",Frame);
//		Sound_effect.clear();//=0x02,
printf(" \n Flash_effect %d",Flash_effect);
printf(" \n Green_component %d",Green_component);
printf(" \n Blue_component %d",Blue_component);
printf(" \n Flash_power %d",Flash_power);
}

void stcAnimationCelldata::clear()
{
Cell_source=0;//=0x02,
X_location=0;//=0x03,
Y_location=0;//=0x04,
Magnification=0;//=0x05,
Red_component=0;//=0x06,
Green_component=0;//=0x07,
Blue_component=0;//=0x08,
Chroma_component=0;//=0x09,
Alpha=0;//=0x0A,
}

void stcAnimationCelldata::show()
{
printf(" \n Cell_source %d",Cell_source);
printf(" \n X_location %d",X_location);
printf(" \n Y_location %d",Y_location);
printf(" \n Magnification %d",Magnification);
printf(" \n Red_component %d",Red_component);
printf(" \n Green_component %d",Green_component);
printf(" \n Blue_component %d",Blue_component);
printf(" \n Chroma_component %d",Chroma_component);
printf(" \n Alpha %d",Alpha);
}

void stcAnimationCell::clear()
{
	Cell_data.clear();
}

void stcAnimated_battle::clear()
{
 strName="";//=0x01,
 strAnimation_file="";//=0x02,
 vecAnimationTiming.clear();//Timing_data=0x06,
 intApply_to=0;//0x09,
  intY_coordinate_line=0;//=0x0A,
 //Framedata=0x0C
}
void stcAnimated_battle::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n strAnimation_file %s",strAnimation_file.c_str());
//vecAnimationTiming.clear();//Timing_data=0x06,
printf(" \n intApply_to %d",intApply_to);
printf(" \n intY_coordinate_line %d",intY_coordinate_line);
 //Framedata=0x0C
}

void stcChipSet::clear()
{
 strName="";//=0x01,
 strGraphic="";//=0x02,
 vc_sh_Lower_tile_terrain.clear();// Lower_tile_terrain=0x03,
 vc_ch_Lower_tile_passable.clear();// Lower_tile_passable=0x04,
 vc_ch_Upper_tile_passable.clear();// Upper_tile_passable=0x05,
 Water_animation=0;//=0x0B,
 Animation_speed=0;//=0x0C
}
void stcChipSet::show()
{
//int i;
printf(" \n strName %s",strName.c_str());
printf(" \n strGraphic %s",strGraphic.c_str());

/*for(i=0; i<vc_sh_Lower_tile_terrain.size() ;i++)
printf(" \n terrain id title %d %d ",i, vc_sh_Lower_tile_terrain[i]);

for(i=0; i<vc_ch_Lower_tile_passable.size() ;i++)
printf(" \n Lower_tile_passable %d %d ",i, vc_ch_Lower_tile_passable[i]);

for(i=0; i<vc_ch_Upper_tile_passable.size() ;i++)
printf(" \n vc_ch_Upper_tile_passable %d %d ",i, vc_ch_Upper_tile_passable[i]);
*/
// vc_ch_Lower_tile_passable.clear();// Lower_tile_passable=0x04,
// vc_ch_Upper_tile_passable.clear();// Upper_tile_passable=0x05,
printf(" \n Water_animation %d",Water_animation);
printf(" \n Animation_speed %d",Animation_speed);
}

void stcGlosary::show()
{
//todas son cadenas
printf(" \n Enemy_encounter %s",Enemy_encounter.c_str());
printf(" \n Headstart_attack %s",Headstart_attack.c_str());
printf(" \n Escape_success %s",Escape_success.c_str());
printf(" \n Escape_failure %s",Escape_failure.c_str());
printf(" \n Battle_victory %s",Battle_victory.c_str());
printf(" \n Battle_defeat %s",Battle_defeat.c_str());
printf(" \n Experience_received %s",Experience_received.c_str());
printf(" \n Money_recieved_A %s",Money_recieved_A.c_str());
printf(" \n Money_recieved_B %s",Money_recieved_B.c_str());
printf(" \n Item_recieved %s",Item_recieved.c_str());
printf(" \n Attack_message %s",Attack_message.c_str());
printf(" \n Ally_critical_hit %s",Ally_critical_hit.c_str());
printf(" \n Enemy_critical_hit %s",Enemy_critical_hit.c_str());
printf(" \n Defend_message %s",Defend_message.c_str());
printf(" \n Watch_message %s",Watch_message.c_str());
printf(" \n Gathering_energy %s",Gathering_energy.c_str());
printf(" \n Sacrificial_attack %s",Sacrificial_attack.c_str());
printf(" \n Enemy_escape %s",Enemy_escape.c_str());
printf(" \n Enemy_transform %s",Enemy_transform.c_str());
printf(" \n Enemy_damaged %s",Enemy_damaged.c_str());
printf(" \n Enemy_undamaged %s",Enemy_undamaged.c_str());
printf(" \n Ally_damaged %s",Ally_damaged.c_str());
printf(" \n Ally_undamaged %s",Ally_undamaged.c_str());
printf(" \n Skill_failure_A %s",Skill_failure_A.c_str());
printf(" \n Skill_failure_B %s",Skill_failure_B.c_str());
printf(" \n Skill_failure_C %s",Skill_failure_C.c_str());
printf(" \n Attack_dodged %s",Attack_dodged.c_str());
printf(" \n Item_use %s",Item_use.c_str());
printf(" \n Stat_recovery %s",Stat_recovery.c_str());
printf(" \n Stat_increase %s",Stat_increase.c_str());
printf(" \n Stat_decrease %s",Stat_decrease.c_str());
printf(" \n Ally_lost_via_absorb %s",Ally_lost_via_absorb.c_str());
printf(" \n Enemy_lost_via_absorb %s",Enemy_lost_via_absorb.c_str());
printf(" \n Resistance_increase %s",Resistance_increase.c_str());
printf(" \n Resistance_decrease %s",Resistance_decrease.c_str());
printf(" \n Level_up_message %s",Level_up_message.c_str());
printf(" \n Skill_learned %s",Skill_learned.c_str());
printf(" \n Shop_greeting %s",Shop_greeting.c_str());
printf(" \n Shop_regreeting %s",Shop_regreeting.c_str());
printf(" \n Buy_message %s",Buy_message.c_str());
printf(" \n Sell_message %s",Sell_message.c_str());
printf(" \n Leave_message %s", Leave_message.c_str());
printf(" \n Buying_message %s",Buying_message.c_str());
printf(" \n Quantity_to_buy %s",Quantity_to_buy.c_str());
printf(" \n Purchase_end %s",Purchase_end.c_str());
printf(" \n Selling_message %s",Selling_message.c_str());
printf(" \n Quantity_to_sell %s",Quantity_to_sell.c_str());
printf(" \n Selling_end %s",Selling_end.c_str());
printf(" \n Shop_greeting_2 %s",Shop_greeting_2.c_str());
printf(" \n Shop_regreeting_2 %s",Shop_regreeting_2.c_str());
printf(" \n Buy_message_2 %s",Buy_message_2.c_str());
printf(" \n Sell_message_2 %s",Sell_message_2.c_str());
printf(" \n Leave_message_2 %s",Leave_message_2.c_str());
printf(" \n Buying_message_2 %s",Buying_message_2.c_str());
printf(" \n Quantity_to_buy_2 %s",Quantity_to_buy_2.c_str());
printf(" \n Purchase_end_2 %s",Purchase_end_2.c_str());
printf(" \n Selling_message_2 %s",Selling_message_2.c_str());
printf(" \n Quantity_to_sell_2 %s",Quantity_to_sell_2.c_str());
printf(" \n Selling_end_2 %s",Selling_end_2.c_str());
printf(" \n Shop_greeting_3 %s",Shop_greeting_3.c_str());
printf(" \n Shop_regreeting_3 %s",Shop_regreeting_3.c_str());
printf(" \n Buy_message_3 %s",Buy_message_3.c_str());
printf(" \n Sell_message_3 %s",Sell_message_3.c_str());
printf(" \n Leave_message_3 %s",Leave_message_3.c_str());
printf(" \n Buying_message_3 %s",Buying_message_3.c_str());
printf(" \n Quantity_to_buy_3 %s",Quantity_to_buy_3.c_str());
printf(" \n Purchase_end_3 %s",Purchase_end_3.c_str());
printf(" \n Selling_message_3 %s",Selling_message_3.c_str());
printf(" \n Quantity_to_sell_3 %s",Quantity_to_sell_3.c_str());
printf(" \n Selling_end_3 %s",Selling_end_3.c_str());
printf(" \n Inn_A_Greeting_A %s",Inn_A_Greeting_A.c_str());
printf(" \n Inn_A_Greeting_B %s",Inn_A_Greeting_B.c_str());
printf(" \n Inn_A_Greeting_C %s",Inn_A_Greeting_C.c_str());
printf(" \n Inn_A_Accept %s",Inn_A_Accept.c_str());
printf(" \n Inn_A_Cancel %s",Inn_A_Cancel.c_str());
printf(" \n Inn_B_Greeting_A %s",Inn_B_Greeting_A.c_str());
printf(" \n Inn_B_Greeting_B %s",Inn_B_Greeting_B.c_str());
printf(" \n Inn_B_Greeting_C %s",Inn_B_Greeting_C.c_str());
printf(" \n Inn_B_Accept %s",Inn_B_Accept.c_str());
printf(" \n Inn_B_Cancel %s",Inn_B_Cancel.c_str());
printf(" \n Loose_items %s",Loose_items.c_str());
printf(" \n Equipped_items %s",Equipped_items.c_str());
printf(" \n Monetary_Unit %s",Monetary_Unit.c_str());
printf(" \n Combat_Command %s",Combat_Command.c_str());
printf(" \n Combat_Auto %s",Combat_Auto.c_str());
printf(" \n Combat_Run %s",Combat_Run.c_str());
printf(" \n Command_Attack %s",Command_Attack.c_str());
printf(" \n Command_Defend %s",Command_Defend.c_str());
printf(" \n Command_Item %s",Command_Item.c_str());
printf(" \n Command_Skill %s",Command_Skill.c_str());
printf(" \n Menu_Equipment %s",Menu_Equipment.c_str());
printf(" \n Menu_Save %s",Menu_Save.c_str());
printf(" \n Menu_Quit %s",Menu_Quit.c_str());
printf(" \n New_Game %s",New_Game.c_str());
printf(" \n Load_Game %s",Load_Game.c_str());
printf(" \n Exit_to_Windows %s",Exit_to_Windows.c_str());
printf(" \n Level %s",Level.c_str());
printf(" \n Health %s",Health.c_str());
printf(" \n Mana %s",Mana.c_str());
printf(" \n Normal_status %s",Normal_status.c_str());
printf(" \n Experience %s",Experience.c_str());
printf(" \n Level_short %s",Level_short.c_str());
printf(" \n Health_short %s",Health_short.c_str());
printf(" \n Mana_short %s",Mana_short.c_str());
printf(" \n Mana_cost %s",Mana_cost.c_str());
printf(" \n Attack %s",Attack.c_str());
printf(" \n Defense %s",Defense.c_str());
printf(" \n Mind %s",Mind.c_str());
printf(" \n Agility %s",Agility.c_str());
printf(" \n Weapon %s",Weapon.c_str());
printf(" \n Shield %s",Shield.c_str());
printf(" \n Armor %s",Armor.c_str());
printf(" \n Helmet %s",Helmet.c_str());
printf(" \n Accessory %s",Accessory.c_str());
printf(" \n Save_game_message %s",Save_game_message.c_str());
printf(" \n Load_game_message %s",Load_game_message.c_str());
printf(" \n Exit_game_message %s",Exit_game_message.c_str());
printf(" \n File_name %s",File_name.c_str());
printf(" \n General_Yes %s",General_Yes.c_str());
printf(" \n General_No %s",General_No.c_str());
}
void stcSystem::show(){
printf(" \n Skiff_graphic %s",Skiff_graphic.c_str());
printf(" \n Boat_graphic %s",Boat_graphic.c_str());
printf(" \n Airship_graphic %s",Airship_graphic.c_str());
printf(" \n Skiff_index %d",Skiff_index);
printf(" \n Boat_index %d",Boat_index);
printf(" \n Airship_index %d",Airship_index);
printf(" \n Title_graphic %s",Title_graphic.c_str());
printf(" \n Game_Over_graphic %s",Game_Over_graphic.c_str());
printf(" \n System_graphic %s",System_graphic.c_str());
printf(" \n Heroes_in_starting %d",Heroes_in_starting);
//std::vector<short> vc_sh_Starting_party;// Starting_party;//0x16,
 Title_music.show();//0x1F,
 Battle_music.show();//0x20,
 Battle_end_music.show();//0x21,
 Inn_music.show();//0x22,
 Skiff_music.show();//0x23,
 Boat_music.show();//0x24,
 Airship_music.show();//0x25,
 Game_Over_music.show();//0x26,
 Cursor_SFX.show();//0x29,
 Accept_SFX.show();//0x2A,
 Cancel_SFX.show();//0x2B,
 Illegal_SFX.show();//0x2C,
 Battle_SFX.show();//0x2D,
 Escape_SFX.show();//0x2E,
 Enemy_attack_SFX.show();//0x2F,
 Enemy_damaged_SFX.show();//0x30,
 Ally_damaged_SFX.show();//0x31,
 Evasion_SFX.show();//0x32,
 Enemy_dead_SFX.show();//0x33,
 Item_use_SFX.show();//0x34,
printf(" \n Map_exit_transition %d",Map_exit_transition);
printf(" \n Map_enter_transition %d",Map_enter_transition);
printf(" \n Battle_start_fadeout %d",Battle_start_fadeout);
printf(" \n Battle_start_fadein %d",Battle_start_fadein);
printf(" \n Battle_end_fadeout %d",Battle_end_fadeout);
printf(" \n Battle_end_fadein %d",Battle_end_fadein);
//0x47 	//no identificados
//0x48	//no identificados
printf(" \n Selected_condition %d",Selected_condition);
printf(" \n Selected_hero %d",Selected_hero);
printf(" \n Battle_test_BG %s",Battle_test_BG.c_str());
//std::string Battle_test_data;//0x55
//0x5B //no identificados

}

void stcBattle_test::show()
{
printf(" \n Hero_ID %d",Hero_ID);
printf(" \n Level %d",Level);
printf(" \n Weapin %d",Weapon);
printf(" \n Shield %d",Shield);
printf(" \n Armor %d",Armor);
printf(" \n Helmet %d",Helmet);
printf(" \n Accessory %d",Accessory);
}

void stcBattle_test::clear()
{
 Hero_ID=0;
 Level=0;// 0x02
 Weapon=0;// 0x0B
 Shield=0;// 0x0C
 Armor=0;// 0x0D
 Helmet=0;// 0x0E
 Accessory=0;// 0x0F
}


void stcEvent::clear()
{
   strName="";//=0x01,
   intNameActivation_condition=0;//=0x0B,
   blNameActivate_on_switch=0;//=0x0C,
   intNameSwitch_ID=0;//=0x0D,
   intNameScript_length=0;//=0x15,
}
void stcEvent::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n intNameActivation_condition %d",intNameActivation_condition);
printf(" \n blNameActivate_on_switch %d",blNameActivate_on_switch);
printf(" \n intNameSwitch_ID %d",intNameSwitch_ID);
printf(" \n intNameScript_length %d",intNameScript_length);
}

void stcCombatcommand::clear()
{
  strName="";// 	=0x01,
  intUsage=0;// 	=0x02,
}
void stcCombatcommand::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n intUsage %d",intUsage);}

void stcCombatcommands::show()//array bidimencional
{ int j,i;
i=vc_Combatcommand.size();
for(j=0;j<i;j++)
  vc_Combatcommand[j].show();

}
void stcCombatcommands::clear()//array bidimencional
{
 vc_Combatcommand.clear();
}





void stcBattle_comand::clear()
{
  strName="";// 	=0x01,
  intMotion_attack=0;// 	=0x02,
  vc_Animated_battle_Combat_Anime.clear();//	=0x0A, //two diminsional arrays
  vc_Animated_battle_Anime_combat_weapons.clear();//=0x0B //two diminsional arrays
}
void stcBattle_comand::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n intMotion_attack %d",intMotion_attack);
//  vc_Animated_battle_Combat_Anime.clear();//	=0x0A, //two diminsional arrays
//  vc_Animated_battle_Anime_combat_weapons.clear();//=0x0B //two diminsional arrays
}

void stcFight_anim::clear()
{
 strName="";//=0x01,
 strFilename="";//=0x02,
 intPosition=0;//=0x03,
 blExtended_expanded=0;//=0x04,
 intID_Expansion_animated_combat_ID=0;//=0x05
}
void stcFight_anim::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n strFilename %s",strFilename.c_str());
printf(" \n intPosition %d",intPosition);
printf(" \n blExtended_expanded %d",blExtended_expanded);
printf(" \n intID_Expansion_animated_combat_ID %d",intID_Expansion_animated_combat_ID);
}


void stcProfetion::clear()
{
strName="";// =0x01,
TwoWeapon=0;//=0x15,
fixed_equipment=0;// =0x16,
AI_forced_action=0;// =0x17,
strong_defense=0;// =0x18,
 //Each_level =0x1F, 2 bytes integer array
vc_sh_Hp.clear();
vc_sh_Mp.clear();
vc_sh_Attack.clear();
vc_sh_Defense.clear();
vc_sh_Mind.clear();
vc_sh_Agility.clear();
Experience_curve_basic_values=0;// =0x29,
Experience_curve_increase_degree=0;// =0x2A,
Experience_curve_correction_value=0;//=0x2B,
Animated_battle=0;//	=0x3E,
skills.clear(); //Special_skills_level=0x3F, misma estructura que heroe
Effectiveness_state_number=0;// =0x47,
vc_ch_Condeffects.clear(); //Effectiveness_state_data =0x48,  //1 byte one dimention array
Effectiveness_Attribute_number=0;// =0x49,
vc_ch_Attribeffects.clear();//Effectiveness_Attribute_data =0x4A,/1 byte one dimention array
vc_int_Combat_Command.clear();  //Combat_Command=0x50 4 bytes one dimention array
}
void stcProfetion::show()
{
printf(" \n strName %s",strName.c_str());
printf(" \n TwoWeapon %d",TwoWeapon);
printf(" \n fixed_equipment %d",fixed_equipment);
printf(" \n AI_forced_action %d",AI_forced_action);
printf(" \n strong_defense %d",strong_defense);
// vc_sh_Hp.clear();
// vc_sh_Mp.clear();
// vc_sh_Attack.clear();
// vc_sh_Defense.clear();
// vc_sh_Mind.clear();
// vc_sh_Agility.clear();
printf(" \n Experience_curve_basic_values %d",Experience_curve_basic_values);
printf(" \n Experience_curve_increase_degree %d",Experience_curve_increase_degree);
printf(" \n Experience_curve_correction_value %d",Experience_curve_correction_value);
printf(" \n Animated_battle %d",Animated_battle);
//skills.clear(); //Special_skills_level=0x3F, misma estructura que heroe
printf(" \n Effectiveness_state_number %d",Effectiveness_state_number);
//vc_ch_Condeffects.clear(); //Effectiveness_state_data =0x48,  //1 byte one dimention array
printf(" \n Effectiveness_Attribute_number %d",Effectiveness_Attribute_number);
//vc_ch_Attribeffects.clear();//Effectiveness_Attribute_data =0x4A,/1 byte one dimention array
//vc_int_Combat_Command.clear();  //Combat_Command=0x50 4 bytes one dimention array
}

