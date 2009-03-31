#ifndef _H_LDB_DATA
#define _H_LDB_DATA

#include "ldbstr.h"

class LDB_data
{
    
public:

    LDB_data();
    ~LDB_data();
    
    std::vector<stcHero*> *heros;
    
    std:: vector <stcSkill*> *skill;
    std:: vector <stcItem*> *items;
    std:: vector <stcEnemy> mosters;
    std:: vector <stcEnemy_group> mosterpartys;
    std:: vector <stcTerrain> Terrains;
    std:: vector <stcAttribute> Attributes;
    std:: vector <stcState> States;
    std:: vector <stcAnimated_battle> Animations;
    std:: vector <stcChipSet> Tilesets;
    stcGlosary Glosary;
    stcSystem System_dat;
    stcCombatcommands Combatcommands;
    std:: vector <stcEvent> Event;
    std:: vector <std::string> Switch_Names;
    std:: vector <std::string> Variable_Names;
    std:: vector <stcProfetion> Professions;
    std:: vector <stcBattle_comand> Fightanims;
};

#endif