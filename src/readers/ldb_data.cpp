#include "ldb_data.h"

LDB_data::LDB_data()
{
    heros = NULL;
    skill = NULL;
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
    
}