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
    std:: vector <stcEnemy*> *monsters;
    std:: vector <stcEnemy_group*> *monsterpartys;
    std:: vector <stcTerrain*> *terrains;
    std:: vector <stcAttribute*> *attributes;
    std:: vector <stcState*> *states;
    std:: vector <stcAnimated_battle*> *animations;
    std:: vector <stcChipSet*> *tilesets;
    stcGlosary *Glosary;
    stcSystem *System_dat;
    stcCombatcommands *Combatcommands;
    std:: vector <stcEvent*> *Event;
    std:: vector <std::string> *Switch_Names;
    std:: vector <std::string> *Variable_Names;
    std:: vector <stcProfetion*> *Professions;
    std:: vector <stcBattle_comand*> *Fightanims;
};

#endif