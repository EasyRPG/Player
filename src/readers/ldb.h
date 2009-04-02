#ifndef LDB_H
#define LDB_H

#include <vector>
#include "ldbchunks.h"
#include "../tools/tools.h"
#include "ldb_data.h"

class LDB_reader
{
public:

    bool Load(std::string Filename, LDB_data * data); // carga
    void ShowInformation(LDB_data * data);

private:

    unsigned char Void;
    tChunk ChunkInfo; // informacion del pedazo leido

    void GetNextChunk(FILE * Stream,LDB_data * data); //lectra de pedasos
    std:: vector <Magicblock> heroskillChunk(FILE * Stream);
    std:: vector <stcHero*> * heroChunk(FILE * Stream);//lectura de pedasos del heroe
    stcSound_effect soundChunk(FILE * Stream);
    std:: vector <stcSkill*> * skillChunk(FILE * Stream);//lectura de pedasos del habilidades
    std:: vector <stcItem*> * itemChunk(FILE * Stream);//lectura de pedasos del objetos
    std:: vector <stcEnemy_Action> mosteractionChunk(FILE * Stream);
    std:: vector <stcEnemy*> * mosterChunk(FILE * Stream);// lectura de monstruos

    std:: vector <stcEnemy_group_data>  MonsterPartyMonsterChunk(FILE * Stream);
    stcEnemy_group_condition MonsterPartyEventconditionChunk(FILE * Stream);
    std:: vector <stcEnemy_group_event_page> MonsterPartyevent_pageChunk(FILE * Stream);

    std:: vector <stcEnemy_group*> * mosterpartyChunk(FILE * Stream);//lectura de party de monster
    std:: vector <stcTerrain*> * terrainChunk(FILE * Stream);// informacion de los terrenos
    std:: vector <stcAttribute*> * attributeChunk(FILE * Stream);// atributos
    std:: vector <stcState*> * statesChunk(FILE * Stream);//estados

    std:: vector <stcAnimationTiming> AnimationTimingChunk(FILE * Stream);
    std:: vector <stcAnimationCell> FramedataChunk(FILE * Stream);
    std:: vector <stcAnimationCelldata> AnimationCelldataChunk(FILE * Stream);

    std:: vector <stcAnimated_battle*> * animationChunk(FILE * Stream);// animacion
    std:: vector <stcChipSet>  & TilesetChunk(FILE * Stream);// titles
    stcGlosary StringChunk(FILE * Stream);
    stcMusic_Background musicChunk(FILE * Stream);
    std::vector <stcBattle_test> Batletest(FILE * Stream);
    stcSystem SystemChunk(FILE * Stream);
    std:: vector <stcEvent> & EventChunk(FILE * Stream);// eventos
    std:: vector <std::string> Switch_VariableChunk(FILE * Stream);// nombres de variables y swiches
    std:: vector <stcCombatcommand> Comand_Chunk2(FILE * Stream);// comandos de combate
    stcCombatcommands Comand_Chunk(FILE * Stream);
    std:: vector <stcProfetion> Profession_Chunk(FILE * Stream);
    std:: vector <stcFight_anim> Fightanim_Chunk2(FILE * Stream);
    std:: vector <stcBattle_comand> Fightanim_Chunk(FILE * Stream);

};

#endif
