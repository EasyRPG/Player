#ifndef EVENTST_H
#define EVENTST_H

    #include "../tools/tools.h"
    #include <vector>
    #include "eventchunks.h"

 using namespace std;
   class Event_comand {// no instanciar
  public:
   short Comand;
   int Depth;
   void show();
    };
  class Event_comand_Simple: public Event_comand {
  public:
   void show();
  };
  class Event_comand_Message: public Event_comand {
  public:
   string Text;
   void show();
  };
  class Event_comand_Message_options: public Event_comand {
   public:
    int Transparency;
    int Position;
    int Prevent_Hiding;
    int Allow_parallel;
    void show();
    };
  class Event_comand_Select_face: public Event_comand {
    public:
     string Filename;
     int Index;
     int Place_at_Right;
     int Flip_Image;
   void show(); };
  class Event_comand_Show_choice: public Event_comand {
   public:
    string Text;
    int Cancel_option;
   void show(); };
  class Event_comand_Show_choice_option: public Event_comand {
   public:
    string Text;
    int Choice_number;
   void show(); };
   class Event_comand_Number_input: public Event_comand {
   public:
    int Digits_to_input;
    int variable_to_store;
   void show(); };
   class Event_comand_Change_switch: public Event_comand {
   public:
    int Mode;
    int start_switch;
    int end_switch;
    int toggle_option;
   void show(); };

  class Event_comand_Change_var: public Event_comand {
   public:
    int Mode; //variable  grupo variable idque esta en variable
    int start_switch;
    int end_switch;
    int operation;//set +- * /
    int op_mode;
    int op_data1;
    int op_data2;
   void show(); };
   class Event_comand_Timer_manipulation: public Event_comand {
   public:
    int Set;
    int By_Value;
    int Seconds;
    int Display;
    int Runs_in_Battle;
    int op_data1;
   void show(); };
   class Event_comand_Change_cash_held: public Event_comand {
   public:
    int Add;
    int By_Value;
    int Amount;
   void show(); };
   class Event_comand_Change_inventory: public Event_comand {
   public:
    int Add;
    int By_ID ;
    int Item_ID;
    int By_Count;
    int Count;
   void show(); };
   class Event_comand_Change_party: public Event_comand {
   public:
    int Add;
    int By_ID ;
    int Hero_ID;
   void show(); };
   class Event_comand_Change_experience: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Add;
    int By_Count;
    int Count;
    int Show_Level_Up_Message;
   void show(); };
   class Event_comand_Change_level: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Add;
    int By_Count;
    int Count;
    int Show_Level_Up_Message;
   void show(); };
   class Event_comand_Change_statistics: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Add;
    int Stat;
    int By_Count;
    int Count;
   void show(); };
   class Event_comand_Learn_forget_skill: public Event_comand {
   public:
    int Hero;
    int Hero_ID;
    int Learn;
    int By_Count;
    int Count;
   void show(); };
   class Event_comand_Change_equipment: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Add_Remove;
    int By_Count;
    int Count;
   void show(); };
   class Event_comand_Change_HP: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Add;
    int By_Count;
    int Count;
    int Possible_Death;
   void show(); };
   class Event_comand_Change_MP: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Add;
    int By_Count;
    int Count;
   void show(); };
   class Event_comand_Change_Status: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Inflict;
    int Status_effect;
   void show(); };
   class Event_comand_Full_Recovery: public Event_comand {
   public:
    int All;
    int Hero_ID;
   void show(); };
   class Event_comand_Inflict_Damage: public Event_comand {
   public:
    int All;
    int Hero_ID;
    int Damage;
    int Defense_effect;
    int Mind_effect;
    int Variance;
    int Save_damage_to_var;
    int Var_ID;
   void show(); };

   class Event_comand_Change_Hero_Name: public Event_comand {
   public:
   string New_name;
    int Hero_ID;
   void show(); };
   class Event_comand_Change_Hero_Class: public Event_comand {
   public:
   string strNew_class;
    int Hero_ID;
   void show(); };
   class Event_comand_Change_Hero_Graphic: public Event_comand {
   public:
   string New_graphic;
   int Hero_ID;
   int Sprite_ID;
   int Transparent;
   void show(); };
   class Event_comand_Change_Hero_Face: public Event_comand {
   public:
   string New_graphic;
   int Hero_ID;
   int Face_ID;
   void show(); };
   class Event_comand_Change_Vehicle: public Event_comand {
   public:
   string New_graphic;
   int Vehicle_ID;
   int Sprite_ID;
   void show(); };
   class Event_comand_Change_System_BGM: public Event_comand {
   public:
   string New_BGM;
   int BGM_ID;
   int Fadein_time;
   int Volume;
   int Tempo;
   int Balance;
   void show(); };
   class Event_comand_Change_System_SE: public Event_comand {
   public:
   string New_SE;
   int SE_ID;
   int Volume;
   int Tempo;
   int Balance;
   void show(); };
   class Event_comand_Change_System_GFX: public Event_comand {
   public:
   string New_graphic;
   int Stretch;
   int Gothic_font;
   void show(); };
   class Event_comand_Change_Transition: public Event_comand {
   public:
   int Type;
   int Transition;
   void show(); };

    class Event_comand_Start_Combat: public Event_comand {
   public:
   string Background;
   int Fixed_group;
   int Group_ID;
   int Background_Flag;
   int Escape;
   int Defeat;
   int First_strike;
   void show(); };
    class Event_comand_Call_Shop: public Event_comand {
   public:
   int Style;
   int Message_style;
   int Handler_on_purchase;

   std:: vector <int> Item_IDs;
   void show(); };
    class Event_comand_Call_Inn: public Event_comand {
   public:
   int Style;
   int Message_style;
   int Cost;
   int Handler_on_rest;
   void show(); };
    class Event_comand_Enter_hero_name: public Event_comand {
   public:
   int Hero_ID;
   int Initial_method;
   int Show_initial_name;
   void show(); };
    class Event_comand_Teleport_Party: public Event_comand {
   public:
   int Map_ID;
   int X;
   int Y;
   void show(); };
   class Event_comand_Store_hero_location: public Event_comand {
   public:
   int Map_ID_Var;
   int X_Var;
   int Y_Var;
   void show(); };
   class Event_comand_Recall_to_location: public Event_comand {
   public:
   int Map_ID_Var;
   int X_Var;
   int Y_Var;
   void show(); };

   class Event_comand_Teleport_Vehicle: public Event_comand {
   public:
   int type;
   int Location;
   int Map_ID;
   int X;
   int Y;
   void show(); };
   class Event_comand_Teleport_Event: public Event_comand {
   public:
   int Event_ID;
   int By_value;
   int X;
   int Y;
   void show(); };
   class Event_comand_Swap_Event_Positions: public Event_comand {
   public:
   int First_event;
   int Second_event;
   void show(); };
   class Event_comand_Get_Terrain_ID: public Event_comand {
   public:
   int By_value;
   int X;
   int Y;
   int Variable_to_store;
   void show(); };
   class Event_comand_Get_Event_ID: public Event_comand {
   public:
   int By_value;
   int X;
   int Y;
   int Variable_to_store;
   void show(); };
   class Event_comand_Erase_screen: public Event_comand {
   public:
   int Transition;
   void show(); };
   class Event_comand_Show_screen: public Event_comand {
   public:
   int Show_screen;
   void show(); };
   class Event_comand_Set_screen_tone: public Event_comand {
   public:
   int Red_diffuse;
   int Green_diffuse;
   int Blue_diffuse;
   int Chroma_diffuse;
   int Length;
   int Wait;
   void show(); };
   class Event_comand_Flash_screen: public Event_comand {
   public:
   int Red_diffuse;
   int Green_diffuse;
   int Blue_diffuse;
   int Strength;
   int Length;
   int Wait;
   void show(); };
   class Event_comand_Shake_screen: public Event_comand {
   public:
   int Power;
   int Speed;
   int Length;
   int Wait;
   void show(); };
   class Event_comand_Pan_screen: public Event_comand {
   public:
   int Type;
   int Direction;
   int Distance;
   int Speed;
   int Wait;
   void show(); };
   class Event_comand_Weather_Effects: public Event_comand {
   public:
   int Type;
   int Speed;
   void show(); };
   class Event_comand_Show_Picture: public Event_comand {
   public:
   string Image_file;
   int Picture_ID;
   int By_Value;
   int X;
   int Y;
   int Move_Map;
   int Magnification;
   int Opacity;
   int Use_color_key;
   int Red_diffuse;
   int Green_diffuse;
   int Blue_diffuse;
   int Chroma_diffuse;
   int Effect;
   int Power;
   void show(); };
   class Event_comand_Move_Picture: public Event_comand {
   public:
   int Picture_ID;
   int By_Value;
   int X;
   int Y;
   int Magnification;
   int Opacity;
   int Red_diffuse;
   int Green_diffuse;
   int Blue_diffuse;
   int Chroma_diffuse;
   int Effect;
   int Power;
   int Length;
   int Wait;
   void show(); };
   class Event_comand_Erase_Picture: public Event_comand {
   public:
   int Picture_ID;
   void show(); };
   class Event_comand_Show_Battle_Anim: public Event_comand {
   public:
   int Animation_ID;
   int Target;
   int Wait;
   int Full_screen;
   void show(); };
   class Event_comand_Set_hero_opacity: public Event_comand {
   public:
   int Opacity;
   void show(); };
   class Event_comand_Flash_event: public Event_comand {
   public:
   int Target;
   int Red;
   int Green;
   int Blue;
   int Strength;
   int Length;
   int Wait;
   void show(); };
   class Event_comand_Move_event: public Event_comand {
   public:
   int Target;
   int Frequency;
   int Repeat_actions;
   int Ignore_impossible;
   std:: vector <Event_comand *> comand_moves;
   void show();
   };
   class Event_comand_Wait: public Event_comand {
   public:
   int Length;
   void show(); };
   class Event_comand_Play_BGM: public Event_comand {
   public:
   string BGM_name;
   int Fade_in_time;
   int Volume;
   int Tempo;
   int Balance;
   void show(); };
   class Event_comand_Fade_out_BGM: public Event_comand {
   public:
   int Fade_in_time;
   void show(); };
   class Event_comand_Play_SE: public Event_comand {
   public:
   string SE_name;
   int Volume;
   int Tempo;
   int Balance;
   void show(); };
   class Event_comand_Play_movie: public Event_comand {
   public:
   string Movie_file;
   int By_value;
   int X;
   int Y;
   int Width;
   int Height;
   void show(); };

   class Event_comand_Key_input: public Event_comand {
   public:
   int Variable_to_store;
   int Wait_for_key;
   int Directionals;
   int Accept;
   int Cancel;
   void show(); };
   class Event_comand_Change_tile: public Event_comand {
   public:
   int New_tile;
   void show(); };
   class Event_comand_Change_background: public Event_comand {
   public:
   string Parallax_BG;
   int X_pan;
   int Y_pan;
   int X_auto_pan;
   int X_pan_speed;
   int Y_auto_pan;
   int Y_pan_speed;
   void show(); };
 class Event_comand_Change_encounter_rate: public Event_comand {
    public:
       int Encounter_rate;
   void show(); };
 class Event_comand_Change_single_tile: public Event_comand {
    public:
       int Layer;
       int Old_tile;
       int New_tile;
   void show(); };
 class Event_comand_Set_teleport_location: public Event_comand {
    public:
       int Add;
       int Map_ID;
       int X;
       int Y;
       int Switch;
       int Switch_ID;
   void show(); };

 class Event_comand_Enable_teleport: public Event_comand {
    public:
       int Enable;
   void show(); };

 class Event_comand_Set_escape_location: public Event_comand {
    public:
       int Map_ID;
       int X;
       int Y;
       int Switch;
       int Switch_ID;
   void show(); };
 class Event_comand_Enable_escape: public Event_comand {
    public:
       int Enable;
   void show(); };
 class Event_comand_Enable_saving: public Event_comand {
    public:
       int Enable;
   void show(); };
 class Event_comand_Enable_system_menu: public Event_comand {
    public:
       int Enable;
   void show(); };
 class Event_comand_Conditional: public Event_comand {
   public:// por confirmar
    int Swich;
    int Hero_ID;
    int Tempo;
    int By_Count;
    int Count;
    int Music;
   void show(); };
 class Event_comand_Label: public Event_comand {
    public:
       int Label_id;
   void show(); };
 class Event_comand_Go_to_label: public Event_comand {
    public:
       int Label_id;
   void show(); };
 class Event_comand_Call_event: public Event_comand {
    public:
       int Method;
       int Event_ID;
       int Event_page;
   void show(); };
  class Event_comand_Comment_Text: public Event_comand {
  public:
   string Text;
  void show(); };
  class Event_comand_Add_line_to_comment: public Event_comand {
  public:
   string Text;
  void show(); };

  class Event_comand_Change_Profetion: public Event_comand {
  public:
//7 datos falta 1
    int Hero_ID;
    int Comands;
    int Levels;
    int Skills;
    int Values;
    int Options;
  void show(); };
   class Event_comand_Change_Batle_Comands: public Event_comand {
  public:
    int remove;
    int Hero_ID;
    int Batle_Comand;
    int Add;
  void show(); };

     struct stEvent
    {
        tChunk ChunkInfo; // informacion del pedazo leido
        Event_comand * EventcommandMessageChunk(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandMessageoptionsChunk(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandSelectfaceChunk(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandShowchoiceChunk(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandSimpleEvent_comand(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandShow_choice_option(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandNumber_input(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_switch(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_var(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandTimer_manipulation(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_cash_held(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_inventory(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_party(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_experience(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_level(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_statistics(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandLearn_forget_skill(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_equipment(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_HP(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_MP(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Status(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandFull_Recovery(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandInflict_Damage(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Hero_Name(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Hero_Class(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Hero_Graphic(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Hero_Face(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Vehicle(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_System_BGM(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_System_SE(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_System_GFX(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Transition(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandStart_Combat(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandCall_Shop(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandCall_Inn(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandEnter_hero_name(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandTeleport_Party(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandStore_hero_location(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandRecall_to_location(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandTeleport_Vehicle(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandTeleport_Event(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandSwap_Event_Positions(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandGet_Terrain_ID(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandGet_Event_ID(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandErase_screen(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandShow_screen(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandSet_screen_tone(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandFlash_screen(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandShake_screen(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandPan_screen(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandWeather_Effects(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandShow_Picture(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandMove_Picture(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandErase_Picture(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandShow_Battle_Anim(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandSet_hero_opacity(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandFlash_event(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandMove_event(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandWait(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandPlay_BGM(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandFade_out_BGM(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandPlay_SE(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandPlay_movie(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandKey_input(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_tile(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_background(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_encounter_rate(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_single_tile(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandSet_teleport_location(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandEnable_teleport(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandSet_escape_location(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandEnable_escape(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandEnable_saving(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandEnable_system_menu(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandConditional(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandLabel(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandGo_to_label(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandCall_event(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandComment_Text(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandAdd_line_to_comment(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Profetion(int Command,int Depth,FILE * Stream);
        Event_comand * EventcommandChange_Batle_Comands(int Command,int Depth,FILE * Stream);

        std:: vector <Event_comand * > EventcommandChunk(FILE * Stream);//instrucciones de pagina de evento
};
#endif
