#ifndef _H_RPG_CACHE_
#define _H_RPG_CACHE_

#include "bitmap.h"
#include <vector>

#define F_BACKDROP "Backdrop/"
#define F_BATTLE "Battle/"
#define F_CHARSET "CharSet/"
#define F_CHIPSET "ChipSet/"
#define F_FACESET "FaceSet/"
#define F_GAMEOVER "GameOver/"
#define F_MONSTER "Monster/"
#define F_MOVIE "Movie/"
#define F_MUSIC "Music/"
#define F_PANORAMA "Panorama/"
#define F_PICTURE "Picture/"
#define F_SOUND "Sound/"
#define F_SYSTEM "System/"
#define F_TITLE "Title/"

class CCache {
public:

    enum res_type {
        BACKDROP = 0,
        BATTLE,
        CHARSET,
        CHIPSET,
        FACESET,
        GAMEOVER,
        MONSTER,
        MOVIE,
        MUSIC,
        PANORAMA,
        PICTURE,
        SOUND,
        SYSTEM,
        TITLE
    };

    CCache():
        system_count(0),
        title_count(0) {}

    ~CCache() {}

    Bitmap* windowskin(int id);
    Bitmap* title(int id);

    int load_graphic(const std::string& filename, res_type type);

private:

    int system_count;
    int title_count;

    std::vector<Bitmap*> windowskins;
    Bitmap* _title;

    //void load_bitmap(std::string& s);

};

namespace RPG {
    extern CCache Cache;
}
#endif
