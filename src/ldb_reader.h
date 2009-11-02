#ifndef _H_LDBREADER_
#define _H_LDBREADER_

#include "ldbchunks.h"
#include "main_data.h"
#include <string>
#include <iostream>
#include "tools.h"
#include "ldb_data.h"

using namespace RPG;

namespace LDB_reader {
    bool load(const std::string& Filename);
    void heroskillChunk(FILE * Stream, Actor* hero);
    void heroChunk(FILE * Stream);
    void skillChunk(FILE * Stream);
    void GetNextChunk(FILE * Stream);
    void soundChunk(FILE * Stream, Skill* sk);
    void itemChunk(FILE * Stream);
    void mosteractionChunk(FILE * Stream, Enemy* e);
    void mosterChunk(FILE * Stream);
    void MonsterPartyMonsterChunk(FILE * Stream, Troop *trp);
    void MonsterPartyEventconditionChunk(FILE * Stream, Page* pgs);
    void MonsterPartyevent_pageChunk(FILE * Stream, Troop *trp);
    void mosterpartyChunk(FILE * Stream);
    void terrainChunk(FILE * Stream);
    void attributeChunk(FILE * Stream);
    void statesChunk(FILE * Stream);
}

#endif
