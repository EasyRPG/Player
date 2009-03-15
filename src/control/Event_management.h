
#ifndef GENERAL_DATA_H_
#define GENERAL_DATA_H_


#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include "../sprites/sprite.h"
#include "../sprites/Animation_Manager.h"
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
#include "../attributes/Player_Team.h"
#include "../tools/math-sll.h"
#include "../attributes/CActor.h"
#include "events/message.h"

class General_data;
class Mv_management;


class E_management
{

private:
    unsigned char *NScene;
    bool *running;
    bool use_keyboard;
    Audio *myaudio;
    General_data *myteam;
    std:: vector <stEventMap> *Events;//agregar apuntador a vector de eventos
    std:: vector <E_state> *Ev_state;//agregar apuntador a vector de eventos
    std:: vector <Chara> *Charas_nps;
    std:: vector <Sprite> images;
    Animacion_Manager On_map_anim;

    Sprite X;
    CMessage *message_box;
    Mv_management * Mov_management;
    CActor *Actor;
    map_data *data;
    Chipset *chip;
    Pre_Chipset * pre_chip;
    bool is_Active(stPageConditionEventMap * Page_conditions);//page conditions
    bool state_swich( int number);
public:
    int Active_page(stEventMap * Event);
    E_management();
    void dispose();
    //static bool is_talking() { return tried_to_talk; }
    void page_refresh();

    static bool tried_to_talk;
    int busque_real_id(int id_to_serch);
    void init(Audio * audio,unsigned char * TheScene,General_data * TheTeam,std:: vector <stEventMap> * TheEvents,CActor * TheActor,Mv_management * Move_management);
    void exec_comand(std:: vector <Event_comand *> vcEvent_comand,int event_id, E_state * comand_id);
    void active_exec_comand(Event_comand * comand,int event_id, E_state * comand_id);
    void update(SDL_Surface *Screen);
    void updatekey(bool *running);
};
#include "General_data.h"


#endif
