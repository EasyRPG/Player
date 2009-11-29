#include "rpg_cache.h"

Bitmap* CCache::windowskin(int id) {
    return windowskins.at(id);
}

Bitmap* CCache::title(int id) {
    return _title;
}

int CCache::load_graphic(const std::string& filename, res_type type)
{
    std::string name;
    int id = 0;

    switch (type) {
        case BACKDROP:
            name.assign(F_BACKDROP + filename);
            break;
        case BATTLE:
            name.assign(F_BATTLE + filename);
            break;
        case CHARSET:
            name.assign(F_CHARSET + filename);
            break;
        case CHIPSET:
            name.assign(F_CHIPSET + filename);
            break;
        case FACESET:
            name.assign(F_FACESET + filename);
            break;
        case GAMEOVER:
            name.assign(F_GAMEOVER + filename);
            break;
        case MONSTER:
            name.assign(F_MONSTER + filename);
            break;
        case MOVIE:
            name.assign(F_MOVIE + filename);
            break;
        case MUSIC:
            name.assign(F_MUSIC + filename);
            break;
        case PANORAMA:
            name.assign(F_PANORAMA + filename);
            break;
        case PICTURE:
            name.assign(F_PICTURE + filename);
            break;
        case SOUND:
            name.assign(F_SOUND + filename);
            break;
        case TITLE:
            id = title_count;
            name.assign(F_TITLE + filename);
            break;
        case SYSTEM:
            id = system_count;
            name.assign(F_SYSTEM + filename);
            break;
        default:
            return -1;
    }

    Bitmap* btmp;
    btmp = new Bitmap(name, id);
    
    /* Check if the constructor actually failed */
    if (btmp->is_zombie()) {
        delete btmp;
        return -1;
    }

    switch (type) {
        case BACKDROP:
            break;
        case BATTLE:
            break;
        case CHARSET:
            break;
        case CHIPSET:
            break;
        case FACESET:
            break;
        case GAMEOVER:
            break;
        case MONSTER:
            break;
        case MOVIE:
            break;
        case MUSIC:
            break;
        case PANORAMA:
            break;
        case PICTURE:
            break;
        case SOUND:
            break;
        case TITLE:
            _title = btmp;
            title_count++;
            break;
        case SYSTEM:
            windowskins.push_back(btmp);
            system_count++;
            break;
        default:
            ;
    }
    return id;
}

namespace RPG {
    CCache Cache;
}