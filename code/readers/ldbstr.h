#ifndef LDBSTR_H
#define LDBSTR_H
#include <string>
#include <vector>
#include "stevent.h"  
struct stcSound_effect{
std::string Name_of_Sound_effect;
int Volume; 
int Tempo; 
int Balance;
void clear();void show();
};

struct stcMusic_Background{
std::string Name_of_Music_Background;
int Fade_in_time;
int Volume; 
int Tempo; 
int Balance;
void clear();void show();
};

struct Magicblock
{
int Level;//=0x01,
int Spell_ID;//=0x02
void clear();void show();
};
struct stcHero
{
std::string strName;// =0x01,
std::string strClass;// =0x02,
std::string strGraphicfile;// =0x03,
int intGraphicindex;// =0x04,
int intTransparent;// =0x05,
int intStartlevel;// =0x07,
int intMaxlevel;// =0x08,
int intCrithit;// =0x09,
int intHitchance;// =0x0A,
std::string strFacegraphic;// =0x0F, 
int intFaceindex;// =0x10,
char blDualwield;// =0x15,
char blFixedequipment;// =0x16,
char blAI;// =0x17,
char blHighdefense;// =0x18,
// Statisticscurves =0x1F, arraglo de 16 bites con 5 datos
std::vector<short> vc_sh_Hp;
std::vector<short> vc_sh_Mp;
std::vector<short> vc_sh_Attack;
std::vector<short> vc_sh_Defense;
std::vector<short> vc_sh_Mind;
std::vector<short> vc_sh_Agility;


int intEXPBaseline;// =0x29,
int intEXPAdditional;// =0x2A,
int intEXPCorrection;// =0x2B,
int intprofesion;// 0x39--2003
int Animatedbattle;// 0x3E--2003
 //Startequip =0x33,arraglo de 16 bites con 5 datos
 
short sh_Weapon;//
short sh_Shield;//
short sh_Armor;//
short sh_Head;//
short sh_Accessory;//
 
std:: vector <Magicblock> skills; //Skills =0x3F,
bool blRenameMagic;// =0x42,
std::string strMagicname;// =0x43,
int intCondlength;// =0x47,
 //array de un byte
std::vector<char> vc_ch_Condeffects;
int intAttriblength;// =0x49,
//array de un byte
std::vector<char> vc_ch_Attribeffects;
std::vector<int> vc_int_Combat_Command; //Combat_Command=0x50 4 bytes one dimention array//Combat Comand // =0x50 //4 byte one byte dimension array of 6 elements
void clear();
void show();
};


struct stcSkill{
std::string  strName;//0x01,
std::string  strDescription;//0x02,
std::string  strtext;//0x03,
std::string  strtextline;//0x04,
int  intFailure;//0x07,
int  intSpelltype;//0x08,
int  intMpconType;//0x09    //2003
int  intConsumtion;//0x0A  //2003
int  intCost;//0x0B,
int  intRange;//0x0C,
int  intSwitch;//0x0D,
int  intBattleanimation;//0x0E,
stcSound_effect  Soundeffect;//0x10,
char blFieldusage;//0x12,
char blCombatusage;//0x13,
char blChangeofstatus;//0x14 //-2003
int  intStrengtheffect;//0x15,
int  intMindeffect;//0x16,
int  intVariance;//0x17,
int  intBasevalue;//0x18,
int  intSuccessrate;//0x19,
char  blAffectHP;//0x1F,
char  blAffectMP;//0x20,
char  blAffectAttack;//0x21,
char  blAffectDefense;//0x22,
char  blAffectMind;//0x23,
char  blAffectAgility;//0x24,
char  blAbsorbdamage;//0x25,
char  blIgnoredefense;//0x26,

int  intConditionslength;//0x29,
std::vector<char> vc_ch_Condeffects;//0x2A,
int  intAttributeslength;//0x2B,
std::vector<char> vc_ch_Attribeffects; //0x2C,

char  blAffectresistance;//0x2D       
int intwhenusinganimation; //0x31 --2003
//CBA data  //0x32
void clear();
void show();
};

struct stcItem
{
std::string Name;//0x01,
std::string Description;//0x02,
int Type;//0x03,
int Cost;//0x05,
int Uses;//0x06,
int Attack;//0x0B,
int Defense;//0x0C,
int Mind;//0x0D,
int Speed;//0x0E,
int Equip;//0x0F,
int MPcost;//0x10,
int Chancetohit;//0x11,
int Criticalhit;//0x12,
int Battleanimation;//0x14,
char Preemptiveattack;//0x15,
char Doubleattack;//0x16,
char Attackallenemies;//0x17,
char Ignoreevasion;//0x18,
char Preventcriticalhits;//0x19,
char Raiseevasion;//0x1A,
char MPusecutinhalf;//0x1B,
char Noterraindamage;//0x1C,
char Is_equip; //0x1D   //2003
int Healsparty;//0x1F,
int HPrecovery;//0x20,
int HPrecoveryvalue;//0x21,
int MPrecovery;//0x22,
int MPrecoveryvalue;//0x23,
char Useonfieldonly;//0x25,
char Onlyondeadheros;//0x26,
int MaxHPmodify;//0x29,
int MaxMPmodify;//0x2A,
int Attackmodify;//0x2B,
int Defensemodify;//0x2C,
int Mindmodify;//0x2D,
int Speedmodify;//0x2E,
int Usagemessage;//0x33,
int Efectiveastool; //0x35 //2003
int Switchtoturnon;//0x37,
char Useonfield;//0x39,
char Useinbattle;//0x3A,

int Heroeslength;//0x3D,
std::vector<char> vc_ch_Heroescanuse;//0x3E,
int Conditionslength;//0x3F,
std::vector<char> vc_ch_Condeffects;//0x40,
int Attributeslength;//0x41,
std::vector<char> vc_ch_Attribeffects; //0x42,


int Chancetochange;//0x43
int Whenuseanimation;//0x45
int ItemtoSkill;//0x47
int EquitoVoc;//0x48
//Vocation possible  1 byte one dimension array//0x49  
void clear();
void show();
};
	
struct stcEnemy
{  
  std::string strName;	// = 0x01,
  std::string strGraphicfile;	//= 0x02,
  int intHuealteration;	//= 0x03,
  int intMaxHP;	//= 0x04,
  int intMaxMP;	//= 0x05,
  int intAttack;	//= 0x06,
  int intDefense;	//= 0x07,
  int intMind;	//= 0x08,
  int intSpeed;	//= 0x09,
  char blTranslucentgraphic;	//= 0x0A,
  int intExperience;	//= 0x0B,
  int intGold;	//= 0x0C,
  int intSpoilsitemID;	//= 0x0D,
  int intSpoilschance;	//= 0x0E,
  char blCanusecriticalhits;	//= 0x15,
  int intCriticalhitchance;	//= 0x16,
  char blUsuallymiss;	//= 0x1A,
  char blAirborne;	//= 0x1C,
  int intConditionslength;	//= 0x1F,
    std::vector<char> vc_ch_Condeffects;//0x20, arreglo de un byte //una dimencion
  int inyAttributeslength;//= 0x21,
 std::vector<char> vc_ch_Attribeffects; // 0x22, arreglo de un byte //una dimencion

  //Actionslist= 0x2A
  void clear();
void show();
};
struct stcEnemy_Action
{  		
 int intAction;//=0x01,	
 int intAction_data;//	=0x02,
 int intSkill_ID;//=0x03,	
 int intMonster_ID;//=0x04,	
 int intCondition;//	=0x05,
 int intLower_limit;//=0x06,	
 int intUpper_limit;//	=0x07,
 int intPriority;//=0x0D
 void clear();
void show();	
};
	

struct stcEnemy_group_data
{ 		
int	Enemy_ID;//=0x01,
int	X_position;//=0x02,
int	Y_position;//=0x03
void clear();
void show();
};

struct stcEnemy_group_condition
{ 		
int Condition_flags;//=0x01,
int Switch_A;//=0x02,
int Turn_number_A;//=0x06,
int Lower_limit;//=0x0B,
int Upper_limit;//=0x0C,
void clear();
void show();	
};
struct stcEnemy_group_event_page
{ 		
stcEnemy_group_condition conditions;//	Page_conditions=0x02,
int	Event_length;//=0x0B,
 std:: vector <Event_comand> vcEvent_comand;//	Event=0x0C
void clear();
void show();
};



struct stcEnemy_group
{   
std::string strName;// =0x01,
std:: vector <stcEnemy_group_data> Enemy_data;//data=0x02,
int intTerrainlength;		//=0x04,
std::vector<char> vc_ch_Terraindata; //=0x05,
std::vector <stcEnemy_group_event_page> vecPartyMonsterevent;//eventpages=0x0B
void clear();
void show();
};

struct stcTerrain
{   
std::string strName;	//=0x01,
int  intDamageontravel;	//=0x02,
int  intEncounterate;	//=0x03,
std::string strBattlebackground;	//=0x04,
char  blSkiffmaypass;	//=0x05,
char  blBoatmaypass;	//=0x06,
char  blAirshipmaypass;	//=0x07,
char  blAirshipmayland;	//=0x09,
int  intHeroopacity;	//=0x0B
///------ 2003 info por confirmar
void clear();
void show();
};

struct stcAttribute
{ 
std::string strName;//=0x01,
int  intType;//=0x02,
int  intA_damage;//=0x0B,
int  intB_damage;//=0x0C,
int  intC_damage;//=0x0F,
int  intD_damage;//e=0x0F,
int  intE_damage;//=0x0F
void clear();
void show();
 };
 
struct stcState
{
 std::string strName;//=0x01,
 int  intLength;//=0x02,
 int  intColor;//=0x03,
 int  intPriority;//=0x04,
 int  intLimitation;//=0x05,
 int  intA_chance;//=0x0B,
 int  intB_chance;//=0x0C,
 int  intC_chance;//=0x0D,
 int  intD_chance;//=0x0E,
 int  intE_chance;//=0x0F,
 int  intTurnsforhealing;//=0x15,
 int  intHealperturn;//=0x16,
 int  intHealonshock;//=0x17,
 int   intTypeofincrease;//0x1E -2003
 char  blHalveAttack;//=0x1F,
 char  blHalveDefense;//=0x20,
 char  blHalveMind;//=0x21,
 char  blHalveAgility;//=0x22,
 int  intHitratechange;//=0x23,
 char blAvoidatack;//0x24 --2003
 char blmagicreflection;//0x25 --2003
 char blItemequip;//0x26 --2003
 int  intAnime;//0x27 --2003
 char  blPrevent_skill_use;//=0x29,
 int  intMinimum_skill_level;//=0x2A,
 char  blPreventmagicuse;//=0x2B,
 int  intMinimummindlevel;//=0x2C,
 int intmpdecreasetype;//0x2D -2003
 int inthpdecreasetype;//0x2E -2003
 std::string strAllyenterstate;//=0x33,
 std::string strEnemyentersstate;//=0x34,
 std::string strAlreadyinstate;//=0x35,
 std::string strAffectedbystate;//=0x36,
 std::string strStatusrecovered;//=0x37,
 int  intHPloss;//=0x3D,
 int  intHPlossvalue;//=0x3E,
 int  intHPmaploss;//=0x3F,
 int  intHPmapsteps;//=0x40,
 int  intMPloss;//=0x41,
 int  intMPlossvalue;//=0x42,
 int  intMPmaploss;//=0x43,
 int  intMPmapsteps;//=0x44
 void clear();
 void show();
};
struct stcAnimationTiming
{	
int Frame;//=0x01,
stcSound_effect	Sound_effect;//=0x02,
int Flash_effect;//=0x03,
int Green_component;//=0x05,
int Blue_component;//=0x06,
int Flash_power;//=0x07
void clear();
void show();    
};
struct stcAnimationCelldata
{	
int Cell_source;//=0x02,
int X_location;//=0x03,
int Y_location;//=0x04,
int Magnification;//=0x05,
int Red_component;//=0x06,
int Green_component;//=0x07,
int Blue_component;//=0x08,
int Chroma_component;//=0x09,
int Alpha;//=0x0A,
void clear();
void show(); 
};
struct stcAnimationCell
{	
 std:: vector <stcAnimationCelldata> Cell_data;   
 void clear();	
};
struct stcAnimated_battle 
{
std::string strName;//=0x01,
std::string strAnimation_file;//=0x02,
std::vector <stcAnimationTiming> vecAnimationTiming; //Timing_data=0x06,
int intApply_to;//0x09,
int  intY_coordinate_line;//=0x0A,
std:: vector <stcAnimationCell>Framedata; //Framedata=0x0C
void clear();
void show(); 
};
	
struct stcChipSet
{  
std::string strName;//=0x01,
std::string strGraphic;//=0x02,
std::vector<short> vc_sh_Lower_tile_terrain;// Lower_tile_terrain=0x03,
std::vector<char> vc_ch_Lower_tile_passable;// Lower_tile_passable=0x04,
std::vector<char> vc_ch_Upper_tile_passable;// Upper_tile_passable=0x05,
int Water_animation;//=0x0B,
int Animation_speed;//=0x0C
void clear();
void show(); 
};
struct stcGlosary{
//todas son cadenas
std::string Enemy_encounter;//0x01,
std::string Headstart_attack;//0x02,
std::string Escape_success;//0x03,
std::string Escape_failure;//0x04,
std::string Battle_victory;//0x05,
std::string Battle_defeat;//0x06,
std::string Experience_received;//0x07,
std::string Money_recieved_A;//0x08,
std::string Money_recieved_B;//0x09,
std::string Item_recieved;//0x0A,
std::string Attack_message;//0x0B,
std::string Ally_critical_hit;//0x0C,
std::string Enemy_critical_hit;//0x0D,
std::string Defend_message;//0x0E,
std::string Watch_message;//0x0F,
std::string Gathering_energy;//0x10,
std::string Sacrificial_attack;//0x11,
std::string Enemy_escape;//0x12,
std::string Enemy_transform;//0x13,
std::string Enemy_damaged;//0x14,
std::string Enemy_undamaged;//0x15,
std::string Ally_damaged;//0x16,
std::string Ally_undamaged;//0x17,
std::string Skill_failure_A;//0x18,
std::string Skill_failure_B;//0x19,
std::string Skill_failure_C;//0x1A,
std::string Attack_dodged;//0x1B,
std::string Item_use;//0x1C,
std::string Stat_recovery;//0x1D,
std::string Stat_increase;//0x1E,
std::string Stat_decrease;//0x1F,
std::string Ally_lost_via_absorb;//0x20,
std::string Enemy_lost_via_absorb;//0x21,
std::string Resistance_increase;//0x22,
std::string Resistance_decrease;//0x23,
std::string Level_up_message;//0x24,
std::string Skill_learned;//0x25,
std::string Shop_greeting ;//0x29,
std::string Shop_regreeting ;//0x2A,
std::string Buy_message ;//0x2B,
std::string Sell_message ;//0x2C,
std::string Leave_message ;//0x2D,
std::string Buying_message ;//0x2E,
std::string Quantity_to_buy ;//0x2F,
std::string Purchase_end ;//0x30,
std::string Selling_message ;//0x31,
std::string Quantity_to_sell ;//0x32,
std::string Selling_end ;//0x33,
std::string Shop_greeting_2 ;//0x36,
std::string Shop_regreeting_2 ;//0x37,
std::string Buy_message_2 ;//0x38,
std::string Sell_message_2 ;//0x39,
std::string Leave_message_2 ;//0x3A,
std::string Buying_message_2 ;//0x3B,
std::string Quantity_to_buy_2 ;//0x3C,
std::string Purchase_end_2 ;//0x3D,
std::string Selling_message_2 ;//0x3E,
std::string Quantity_to_sell_2 ;//0x3F,
std::string Selling_end_2 ;//0x40,
std::string Shop_greeting_3 ;//0x43,
std::string Shop_regreeting_3 ;//0x44,
std::string Buy_message_3 ;//0x45,
std::string Sell_message_3 ;//0x46,
std::string Leave_message_3 ;//0x47,
std::string Buying_message_3 ;//0x48,
std::string Quantity_to_buy_3 ;//0x49,
std::string Purchase_end_3 ;//0x4A,
std::string Selling_message_3 ;//0x4B,
std::string Quantity_to_sell_3 ;//0x4C,
std::string Selling_end_3 ;//0x4D,
std::string Inn_A_Greeting_A;//0x50,
std::string Inn_A_Greeting_B;//0x51,
std::string Inn_A_Greeting_C;//0x52,
std::string Inn_A_Accept;//0x53,
std::string Inn_A_Cancel;//0x54,
std::string Inn_B_Greeting_A;//0x55,
std::string Inn_B_Greeting_B;//0x56,
std::string Inn_B_Greeting_C;//0x57,
std::string Inn_B_Accept;//0x58,
std::string Inn_B_Cancel;//0x59,
std::string Loose_items;//0x5C,
std::string Equipped_items;//0x5D,
std::string Monetary_Unit;//0x5F,
std::string Combat_Command;//0x65,
std::string Combat_Auto;//0x66,
std::string Combat_Run;//0x67,
std::string Command_Attack;//0x68,
std::string Command_Defend;//0x69,
std::string Command_Item;//0x6A,
std::string Command_Skill ;//0x6B,
std::string Menu_Equipment;//0x6C,
std::string Menu_Save;//0x6E,
std::string Menu_Quit;//0x70,
std::string New_Game;//0x72,
std::string Load_Game;//0x73,
std::string Exit_to_Windows;//0x75,
std::string Level;//0x7B,
std::string Health;//0x7C,
std::string Mana;//0x7D,
std::string Normal_status;//0x7E,
std::string Experience ;//0x7F,//(short)
std::string Level_short ;//0x80,//(short)
std::string Health_short ;//0x81,//(short)
std::string Mana_short ;//0x82,//(short)
std::string Mana_cost;//0x83,
std::string Attack;//0x84,
std::string Defense;//0x85,
std::string Mind;//0x86,
std::string Agility;//0x87,
std::string Weapon;//0x88,
std::string Shield;//0x89,
std::string Armor;//0x8A,
std::string Helmet;//0x8B,
std::string Accessory;//0x8C,
std::string Save_game_message;//0x92,
std::string Load_game_message;//0x93,
std::string Exit_game_message;//0x94,
std::string File_name;//0x97,
std::string General_Yes;//0x98,
std::string General_No;//0x99
void show(); 
};
struct stcBattle_test	
{
int Hero_ID;	
int Level;// 0x02
int Weapon;// 0x0B
int Shield;// 0x0C
int Armor;// 0x0D
int Helmet;// 0x0E
int Accessory;// 0x0F
 void clear();
 void show(); 
};
      
struct stcSystem{  

int intLDB_ID;//exclusivo 2003
std::string Skiff_graphic;//0x0B,
std::string Boat_graphic;//0x0C,
std::string Airship_graphic;//0x0D,
int Skiff_index;//0x0E,
int Boat_index;//0x0F,
int Airship_index;//0x10,
std::string Title_graphic;//0x11,
std::string Game_Over_graphic;//0x12,
std::string System_graphic;//0x13,
std::string System_graphic_2;//=0x14,//exclusivo 2003
int Heroes_in_starting;//0x15,
std::vector<short> vc_sh_Starting_party;// Starting_party;//0x16,

int intNum_Comadns_order;//=0x1A,// exclusivo 2003
std::vector<short> vc_sh_Comadns_order;//=0x1B,// exclusivo 2003 array 2 bytes

stcMusic_Background Title_music;//0x1F,
stcMusic_Background Battle_music;//0x20,
stcMusic_Background Battle_end_music;//0x21,
stcMusic_Background Inn_music;//0x22,
stcMusic_Background Skiff_music;//0x23,
stcMusic_Background Boat_music;//0x24,
stcMusic_Background Airship_music;//0x25,
stcMusic_Background Game_Over_music;//0x26,
stcSound_effect Cursor_SFX;//0x29,
stcSound_effect Accept_SFX;//0x2A,
stcSound_effect Cancel_SFX;//0x2B,
stcSound_effect Illegal_SFX;//0x2C,
stcSound_effect Battle_SFX;//0x2D,
stcSound_effect Escape_SFX;//0x2E,
stcSound_effect Enemy_attack_SFX;//0x2F,
stcSound_effect Enemy_damaged_SFX;//0x30,
stcSound_effect Ally_damaged_SFX;//0x31,
stcSound_effect Evasion_SFX;//0x32,
stcSound_effect Enemy_dead_SFX;//0x33,
stcSound_effect Item_use_SFX;//0x34,
int Map_exit_transition;//0x3D,
int Map_enter_transition;//0x3E,
int Battle_start_fadeout;//0x3F,
int Battle_start_fadein;//0x40,
int Battle_end_fadeout;//0x41,
int Battle_end_fadein;//0x42,

int Message_background;//=0x47,//nuevo	
int Font;//=0x47,//nuevo

int Selected_condition;//0x51,
int Selected_hero;//0x52,
std::string Battle_test_BG;//0x54,

std::vector <stcBattle_test> vc_Battle_test; //0x55 //nuevo
int Times_saved;//=0x5B,//nuevo    
int Show_frame;//=0x63,	// exclusivo 2003
int In_battle_anim;//=0x65	// exclusivo 2003  
void show(); 
};	
 
struct stcEvent
{   	
 std::string strName;//=0x01,
 int intNameActivation_condition;//=0x0B,
 char blNameActivate_on_switch;//=0x0C,
 int intNameSwitch_ID;//=0x0D,
 int intNameScript_length;//=0x15,
 std:: vector <Event_comand> vcEvent_comand; //Script=0x16
 void clear();
 void show(); 
};

//---------------------exclusivos del 2003
struct stcCombatcommand//array bidimencional
{ 
     std::string strName;//=0x01,
     int intUsage;//=0x02 
 void clear();
 void show(); 	   		 
};

struct stcCombatcommands//array bidimencional
{ 
std:: vector <stcCombatcommand> vc_Combatcommand;
 void clear();
 void show(); 		 
};
struct stcFight_anim
{   
  std::string strName;//=0x01, 
  std::string	strFilename;//=0x02, 
  int intPosition;//=0x03,  
  char blExtended_expanded;//=0x04,  	
  int intID_Expansion_animated_combat_ID;//=0x05
  void clear();
  void show(); 
};

struct stcBattle_comand
{      
  std::string strName;// 	=0x01, 
  int intMotion_attack;// 	=0x02, 
  std::vector<stcFight_anim> vc_Animated_battle_Combat_Anime;//	=0x0A, //two diminsional arrays 
  std::vector<stcFight_anim> vc_Animated_battle_Anime_combat_weapons;//=0x0B //two diminsional arrays 
  void clear();
  void show();    	   		 
};



struct stcProfetion
{
std::string strName;// =0x01,
char TwoWeapon;//=0x15,
char fixed_equipment;// =0x16,
char AI_forced_action;// =0x17,
char strong_defense;// =0x18,
std::vector<short> vc_sh_Hp;
std::vector<short> vc_sh_Mp;
std::vector<short> vc_sh_Attack;
std::vector<short> vc_sh_Defense;
std::vector<short> vc_sh_Mind;
std::vector<short> vc_sh_Agility;
int Experience_curve_basic_values;// =0x29,
int Experience_curve_increase_degree;// =0x2A,
int Experience_curve_correction_value;//=0x2B,
int Animated_battle;//	=0x3E,
std:: vector <Magicblock> skills; //Skills =0x3F,	 //Special_skills_level=0x3F, misma estructura que heroe
int Effectiveness_state_number;// =0x47,
std::vector<char> vc_ch_Condeffects; //Effectiveness_state_data =0x48,  //1 byte one dimention array
int Effectiveness_Attribute_number;// =0x49,
std::vector<char> vc_ch_Attribeffects;  //Effectiveness_Attribute_data =0x4A,/1 byte one dimention array
std::vector<int> vc_int_Combat_Command; //Combat_Command=0x50 4 bytes one dimention array
void clear();
void show(); 
};



#endif
