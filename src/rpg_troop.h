#ifndef __troop__
#define __troop__
#include <string>
#include <vector>

namespace RPG {

typedef struct {
    int enemy_id;
    int x;
    int y;
} Member;

/* TODO: This struct in incomplete. Lacks documentation */
typedef struct {
    int flags;
    int switch_a;
    int turn_number_a;
    int upper_limit;
    int lower_limit;    
} Condition;
/* ---------------------------------------------------- */

typedef struct {
    Condition condition;
    char span; //0, 1, 2
//    std::vector<EventCommand*> list;
} Page;


class Troop {
    public:
        Troop();
        
        int id;
        std::string name;
        std::vector<char> terrain_data;
        std::vector<Member*> members;
        std::vector<Page*> pages;
        
};

}

#endif
