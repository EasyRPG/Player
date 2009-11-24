#ifndef _H_LDBREADER_
#define _H_LDBREADER_

#include "player.h"
#include "ldbchunks.h"
#include "main_data.h"
#include <string>
#include <iostream>
#include "tools.h"
#include "ldb_data.h"

namespace LDB_reader {
    bool load(const std::string& Filename);
    void GetNextChunk(FILE * Stream);
    void itemChunk(FILE * Stream);
    void heroskillChunk(FILE * Stream, RPG::Actor*& hero);
    void heroChunk(FILE * Stream);
    void skillChunk(FILE * Stream);
    void statesChunk(FILE * Stream);
    void terrainChunk(FILE * Stream);
    void MonsterPartyMonsterChunk(FILE * Stream, RPG::Troop*& trp);
    void MonsterPartyEventconditionChunk(FILE * Stream, RPG::Page*& pgs);
    void MonsterPartyevent_pageChunk(FILE * Stream, RPG::Troop*& trp);
    void mosterpartyChunk(FILE * Stream);
    void attributeChunk(FILE * Stream);
    void mosteractionChunk(FILE * Stream, RPG::Enemy*& e);
    void mosterChunk(FILE * Stream);
    void stringChunk(FILE * Stream);
	
	void systemChunk(FILE * Stream);
    
	void soundChunk(FILE * Stream, RPG::Sound*& sound);
	void musicChunk(FILE * Stream, RPG::Music*& music);
}

#endif
