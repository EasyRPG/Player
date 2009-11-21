#include "rpg_troop.h"

RPG::Troop::Troop() {
    id = 0;
    name = "";
}

RPG::Troop::~Troop() {
    int i, l;

    l = members.size();
    for (i = 0; i < l; ++i) {
        delete members[i];
    }

    l = pages.size();
    for (i = 0; i < l; ++i) {
        delete pages[i];
    }

}
