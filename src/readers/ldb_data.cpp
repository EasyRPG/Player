#include "ldb_data.h"

LDB_data::LDB_data()
{
    heros = NULL;
    skill = NULL;
    items = NULL;
    monsters = NULL;
    monsterpartys = NULL;
    terrains = NULL;
    attributes = NULL; 
    states = NULL;
    tilesets = NULL;
    Glosary = NULL;
    System_dat = NULL;
    Combatcommands = NULL;
    Event = NULL;
    Switch_Names = NULL;
    Variable_Names = NULL;
    Professions = NULL;
    Fightanims = NULL;
}

LDB_data::~LDB_data()
{
    int i;
    int l = heros->size();
    
    if (heros != NULL) 
    {
        for (i = 0; i < l; i++)
        {
            delete heros->at(i);
        }
        delete heros;
    }
    
    if (skill != NULL)
    {
        l = skill->size();
        for (i = 0; i < l; i++)
        {
            delete skill->at(i);
        }
        delete skill;
    }
    
    if (items != NULL)
    {
        l = items->size();
        for (i = 0; i < l; i++)
        {
            delete items->at(i);
        }
        delete items;
    }

    if (monsters != NULL)
    {
        l = monsters->size();
        for (i = 0; i < l; i++)
        {
            delete monsters->at(i);
        }
        delete monsters;
    }
    
    if (monsterpartys != NULL)
    {
        l = monsterpartys->size();
        for (i = 0; i < l; i++)
        {
            delete monsterpartys->at(i);
        }
        delete monsterpartys;
    }    
    
    if (terrains != NULL)
    {
        l = terrains->size();
        for (i = 0; i < l; i++)
        {
            delete terrains->at(i);
        }
        delete terrains;        
    }
    
    if (attributes != NULL)
    {
        l = attributes->size();
        for (i = 0; i < l; i++)
        {
            delete attributes->at(i);
        }
        delete attributes;        
    }
    
    if (states != NULL)
    {
        l = states->size();
        for (i = 0; i < l; i++)
        {
            delete states->at(i);
        }
        delete states;        
    }
    
    if (tilesets != NULL)
    {
        l = tilesets->size();
        for (i = 0; i < l; i++)
        {
            delete tilesets->at(i);
        }
        delete tilesets;        
    }
    
    if (Glosary != NULL) delete Glosary;
    if (System_dat != NULL) delete System_dat;
    if (Combatcommands != NULL) delete Combatcommands;
    
    if (Event != NULL)
    {
        l = Event->size();
        for (i = 0; i < l; i++)
        {
            delete Event->at(i);
        }
        delete Event;        
    }
    
    if (Switch_Names != NULL) delete Switch_Names;
    if (Variable_Names != NULL) delete Variable_Names;
    
    if (Professions != NULL)
    {
        l = Professions->size();
        for (i = 0; i < l; i++)
        {
            delete Professions->at(i);
        }
        delete Professions;        
    }
    
    if (Fightanims != NULL)
    {
        l = Fightanims->size();
        for (i = 0; i < l; i++)
        {
            delete Fightanims->at(i);
        }
        delete Fightanims;        
    }
    
    
}