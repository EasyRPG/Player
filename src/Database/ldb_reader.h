#ifndef _H_LDBREADER_
#define _H_LDBREADER_

#include "ldbchunks.h"
#include "actor.h"
#include "../main_data.h"
#include <string>

namespace LDB_reader {
    bool load(std::string Filename);
    void heroskillChunk(FILE * Stream, Actor* hero);
    void heroChunk(FILE * Stream)
}



#endif
