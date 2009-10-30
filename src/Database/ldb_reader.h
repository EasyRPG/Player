#ifndef _H_LDBREADER_
#define _H_LDBREADER_

#include "ldbchunks.h"
#include "actor.h"
#include "../main_data.h"
#include "item.h"
#include "skill.h"
#include "enemy.h"
#include <string>

namespace LDB_reader {
    bool load(std::string& Filename);
    void heroskillChunk(FILE * Stream, Actor* hero);
    void heroChunk(FILE * Stream);
    void skillChunk(FILE * Stream);
    void GetNextChunk(FILE * Stream);
    void soundChunk(FILE * Stream, Skill* sk);
    void itemChunk(FILE * Stream);
    void mosteractionChunk(FILE * Stream, Enemy* e);
    void mosterChunk(FILE * Stream);
}

#endif
