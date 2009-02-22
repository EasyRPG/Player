#ifndef STRMAP_H
#define STRMAP_H

using namespace std;
#include <vector>
#include <string>
#include "stevent.h"

    class Move_comand {//uso de clase por necesidad de herencia
    public:
    char Comand;
    void show();
   };

    class Move_comand_Switch: public Move_comand {
     public:
     int Switch_ID;
      void show();
    };

    class Move_comand_New_Graphic: public Move_comand {
    public:
    string Name_of_graphic;
     void show();
    };

    class Move_comand_Sound_effect: public Move_comand {
    public:
    string Name_of_Sound_effect;
    int Volume;
    int Tempo;
    int Balance;
     void show();
    };

    struct stPageConditionEventMap
    {
    int Conditions;	//0x01
    int Switch_A_ID;	//0x02
    int Switch_B_ID;	//0x03
    int Variable_ID;	//0x04
    int Variable_value;//	0x05
    int Item;	//0x06
    int Hero;	//0x07
    int Clock;//	0x08
    //metodos
    void clear();
    void show();
    };

    struct stPageMovesEventMap
    {
    int Movement_length;//	0x0B
    int Movement_commands;//	0x0C
    std:: vector <Move_comand> vcMovement_commands;
    bool Repeat_movement;	//0x15
    bool Ignore_impossible;//	0x16
    //metodos
    void clear();
    void show();
    };

    struct stPageEventMap//ya
    {
    stPageConditionEventMap Page_conditions;      //  Page conditions	0x02
    string CharsetName; //0x15
    int CharsetID;      //0x16
    int Facing_direction; //0x17
    int Animation_frame; //0x18
    bool Transparency; //0x19
    int Movement_type;//0x1F
    int Movement_frequency;//0x20
    int Activation_condition;//0x21
    int Event_height;//	0x22
    bool event_overlap;//	0x23
    int Animation_type;//	0x24
    int Movement_speed;//	0x25
    stPageMovesEventMap vcPage_Moves; //Movement block	0x29
    int Script_header;	//0x33
    std:: vector <Event_comand *> vcEvent_comand;   //Script code	0x34
    //metodos
    void clear();
    void show();
    };

    struct stEventMap//ya
    {
    int DB_id;
    string EventName;
    int X_position;
    int Y_position;
    std:: vector <stPageEventMap> vcPage;
    //metodos
    void clear();
    void show();
    };
#endif
