#ifndef __e_action__
#define __e_action__

class E_Action {
    public:
        E_Action();
        
        /* TODO: Write some enum here */
        int id;
        int kind;
        int basic;
        int skill_id;
        int monster_id;
        int condition;
        int lower_limit;
        int upper_limit;
        char priority;    
};


#endif
