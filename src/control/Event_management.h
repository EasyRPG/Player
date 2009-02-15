#ifndef EVENT_MANAGEMENT_H_
#define EVENT_MANAGEMENT_H_

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include "../sprites/sprite.h"
#include "../sprites/chipset.h"
#include "../sprites/Pre_Chipset.h"
#include "../tools/key.h"
#include "../tools/font.h"
#include "../tools/audio.h"
#include "../readers/map.h"
#include "../readers/lmt.h"
#include "../readers/ldb.h"
#include <vector>
#include <string>
#include "../interface/Windows/Window_Base.h"
#include "../attributes/skill.h"
#include "../attributes/item.h"
#include "../attributes/Enemy.h"
#include "../attributes/Player.h"
#include "../tools/math-sll.h"
#include "../attributes/CActor.h"
#include "scene.h"
#include "events/message.h"

struct E_state
{

    bool Event_Active;
    unsigned int id_exe_actual;
    bool id_actual_active;
    unsigned int Active_page;

};

class E_management
{

private:
    unsigned char *NScene;
    bool *running;
    bool use_keyboard;
    Audio *myaudio;
    Player_Team *myteam;
    std:: vector <stEventMap> *Events;//agregar apuntador a vector de eventos
    std:: vector <Chara> *Charas_nps;
    CMessage *message_box;
    CActor *Actor;
    map_data *data;
    Chipset *chip;
public:

    E_management();
    void dispose();
    //static bool is_talking() { return tried_to_talk; }

    static bool tried_to_talk;

    void init(Audio * audio,unsigned char * TheScene,Player_Team * TheTeam,std:: vector <stEventMap> * TheEvents, std:: vector <Chara> * TheCharas_nps,CActor * TheActor,map_data * Thedata,Chipset * the_chip);
    void exec_comand(std:: vector <Event_comand *> vcEvent_comand,int event_id, E_state * comand_id);
    void active_exec_comand(Event_comand * comand, E_state * comand_id);
    void update(SDL_Surface *Screen);
    void updatekey(bool *running);
};


#endif
