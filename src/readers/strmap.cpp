    #include "strmap.h"
   void Move_comand_New_Graphic:: show(){
     printf("\nName_of_graphic  %s\n", Name_of_graphic.c_str());//0x15
   }
   void Move_comand_Sound_effect:: show(){
   printf("\nName_of_Sound_effect  %s\n", Name_of_Sound_effect.c_str());
   printf("\nPosition %d ",Volume);
   printf("\nPrevent_Hiding %d ",Tempo);
   printf("\nAllow_parallel %d ",Balance);
   }
   void stPageConditionEventMap::clear()
   {//defaults
     Conditions=0;	//0x01
     Switch_A_ID=1;	//0x02
     Switch_B_ID=1;	//0x03
     Variable_ID=1;	//0x04
     Variable_value=0;//	0x05
     Item=1;	//0x06
     Hero=1;	//0x07
     Clock=0;//	0x08
   }
   void stPageConditionEventMap::show()
   {
      printf("\nConditions %d ",Conditions);//0x01
      printf("\nSwitch_A_ID %d ",Switch_A_ID); //0x02
      printf("\nSwitch_B_ID %d ",Switch_B_ID); //0x03
      printf("\nVariable_ID %d ",Variable_ID); //0x04
      printf("\nVariable_value %d ",Variable_value);//	0x05
      printf("\nItem %d ",Item);//0x06
      printf("\nHero %d ",Hero);//0x07
      printf("\nClock %d ",Clock);//	0x08
   }

     void stPageMovesEventMap::clear()
    { //defaults
    	Movement_length=0;//	0x0B
    	Movement_commands=0;//	0x0C
    	Repeat_movement=1;	//0x15
    	Ignore_impossible=0;//	0x16
    }
    void stPageMovesEventMap::show()
    {
    	printf("\nMovement_length %d ",Movement_length);//	0x0B
    	printf("\nMovement_commands %d ",Movement_commands);//	0x0C
    	printf("\nRepeat_movement %d ",Repeat_movement);//0x15
    	printf("\nIgnore_impossible %d ",Ignore_impossible);//	0x16
        for (unsigned int j=0; j< vcMovement_commands.size();j++)
        {
            printf("\n id en stack %d ",j+1);
          vcMovement_commands[j].show();
        }

    }
    void stPageEventMap::clear()
    {
        Page_conditions.clear();      //  Page conditions	0x02
        CharsetName=""; //0x15
        CharsetID=0;      //0x16
        Facing_direction=2; //0x17
        Animation_frame=1; //0x18
        Transparency=false; //0x19
        Movement_type=0;//0x1F
        Movement_frequency=3;//0x20
        Activation_condition=0;//0x21
        Event_height=0;//	0x22
        event_overlap=0;//	0x23
        Animation_type=0;//	0x24
        Movement_speed=3;//	0x25
        vcPage_Moves.clear(); //Movement block	0x29
        Script_header=0;	//0x33
        vcEvent_comand.clear();
    }


    void Move_comand::show()
    {
       printf("\n comnand id %d ",Comand); //	0x25
    }

    void stPageEventMap::show()
    {  int i;
       Page_conditions.show();      //  Page conditions	0x02
       printf("\nCharsetName  %s\n", CharsetName.c_str());//0x15
       printf("\nCharsetID %d ",CharsetID);//0x16
       printf("\nFacing_direction %d ",Facing_direction);//0x17
       printf("\nAnimation_frame %d ",Animation_frame);//0x18
       printf("\nTransparency %d ",Transparency);//0x19
       printf("\nMovement_type %d ",Movement_type);//0x1F
       printf("\nMovement_frequency %d ",Movement_frequency);//0x20
       printf("\nActivation_condition %d ",Activation_condition); //0x21
       printf("\nEvent_height %d ",Event_height);//	0x22
       printf("\nevent_overlap %d ",event_overlap); //	0x23
       printf("\nAnimation_type %d ",Animation_type);//	0x24
       printf("\nMovement_speed %d ",Movement_speed); //	0x25
       vcPage_Moves.show();
       printf("\nScript_header %d ",Script_header);	//0x33
      i=vcEvent_comand.size();
Event_comand * comand;
      for (int j=0; j<i;j++)
      {   //printf("\n \nComand num %d \n",j+1);
        //  vcEvent_comand[j].show();
        comand=(vcEvent_comand[j]);

    switch (comand->Comand)
    {
    case Message_options:// 0xCF08,
        Event_comand_Message_options * comand_Message_options;
        comand_Message_options = (Event_comand_Message_options *)comand;
        comand_Message_options->show();
        break;
    case Select_message_face:// 0xCF12,
        Event_comand_Select_face * comand_Select_face;
        comand_Select_face = (Event_comand_Select_face *)comand;
        comand_Select_face->show();
        break;
    case Show_choice:// 0xCF1C,
        Event_comand_Show_choice * comand_Show_choice;
        comand_Show_choice = (Event_comand_Show_choice *)comand;
        comand_Show_choice->show();
        break;
    case Show_choice_option:// 0x819D2C,
        Event_comand_Show_choice_option * comand_Show_choice_option;
        comand_Show_choice_option = (Event_comand_Show_choice_option *)comand;
        comand_Show_choice_option->show();
        break;
    case Number_input:// 0xCF26,
        Event_comand_Number_input * comand_Number_input;
        comand_Number_input = (Event_comand_Number_input *)comand;
        comand_Number_input->show();
        break;
    case Change_switch:// 0xCF62,
        Event_comand_Change_switch * comand_Change_switch;
        comand_Change_switch = (Event_comand_Change_switch *)comand;
        comand_Change_switch->show();
        break;
    case Change_var:// 0xCF6C,
        Event_comand_Change_var * comand_Change_var;
        comand_Change_var = (Event_comand_Change_var *)comand;
        comand_Change_var->show();
        break;
    case Timer_manipulation:// 0xCF76,
        Event_comand_Timer_manipulation * comand_Timer_manipulation;
        comand_Timer_manipulation = (Event_comand_Timer_manipulation *)comand;
        comand_Timer_manipulation->show();
        break;
    case Change_cash_held:// 0xD046,
        Event_comand_Change_cash_held * comand_Change_cash_held;
        comand_Change_cash_held = (Event_comand_Change_cash_held *)comand;
        comand_Change_cash_held->show();
        break;
    case Change_inventory:// 0xD050,
        Event_comand_Change_inventory * comand_Change_inventory;
        comand_Change_inventory = (Event_comand_Change_inventory *)comand;
        comand_Change_inventory->show();
        break;
    case Change_party:// 0xD05A,
        Event_comand_Change_party * comand_Change_party;
        comand_Change_party = (Event_comand_Change_party *)comand;
        comand_Change_party->show();
        break;
    case Change_experience:// 0xD12A,
        Event_comand_Change_experience * comand_Change_experience;
        comand_Change_experience = (Event_comand_Change_experience *)comand;
        comand_Change_experience->show();
        break;
    case Change_level:// 0xD134,
        Event_comand_Change_level * comand_Change_level;
        comand_Change_level = (Event_comand_Change_level *)comand;
        comand_Change_level->show();
        break;
    case Change_statistics:// 0xD13E,
        Event_comand_Change_statistics * comand_Change_statistics;
        comand_Change_statistics = (Event_comand_Change_statistics *)comand;
        comand_Change_statistics->show();
        break;
    case Learn_forget_skill:// 0xD148,
        Event_comand_Learn_forget_skill * comand_Learn_forget_skill;
        comand_Learn_forget_skill = (Event_comand_Learn_forget_skill *)comand;
        comand_Learn_forget_skill->show();
        break;
    case Change_equipment:// 0xD152,
        Event_comand_Change_equipment * comand_Change_equipment;
        comand_Change_equipment = (Event_comand_Change_equipment *)comand;
        comand_Change_equipment->show();
        break;
    case Change_HP:// 0xD15C,
        Event_comand_Change_HP * comand_Change_HP;
        comand_Change_HP = (Event_comand_Change_HP *)comand;
        comand_Change_HP->show();
        break;
    case Change_MP:// 0xD166,
        Event_comand_Change_MP * comand_Change_MP;
        comand_Change_MP = (Event_comand_Change_MP *)comand;
        comand_Change_MP->show();
        break;
    case Change_Status:// 0xD170,
        Event_comand_Change_Status * comand_Change_Status;
        comand_Change_Status = (Event_comand_Change_Status *)comand;
        comand_Change_Status->show();
        break;
    case Full_Recovery:// 0xD17A,
        Event_comand_Full_Recovery * comand_Full_Recovery;
        comand_Full_Recovery = (Event_comand_Full_Recovery *)comand;
        comand_Full_Recovery->show();
        break;
    case Inflict_Damage:// 0xD204,
        Event_comand_Inflict_Damage * comand_Inflict_Damage;
        comand_Inflict_Damage = (Event_comand_Inflict_Damage *)comand;
        comand_Inflict_Damage->show();
        break;
    case Change_Hero_Name:// 0xD272,
        Event_comand_Change_Hero_Name * comand_Change_Hero_Name;
        comand_Change_Hero_Name = (Event_comand_Change_Hero_Name * )comand;
        comand_Change_Hero_Name->show();
        break;
    case Change_Hero_Class:// 0xD27C,
        Event_comand_Change_Hero_Class * comand_Change_Hero_Class;
        comand_Change_Hero_Class = (Event_comand_Change_Hero_Class *)comand;
        comand_Change_Hero_Class->show();
        break;
    case Change_Hero_Graphic:// 0xD306,
        Event_comand_Change_Hero_Graphic * comand_Change_Hero_Graphic;
        comand_Change_Hero_Graphic = (Event_comand_Change_Hero_Graphic *)comand;
        comand_Change_Hero_Graphic->show();
        break;
    case Change_Hero_Face:// 0xD310,
        Event_comand_Change_Hero_Face * comand_Change_Hero_Face;
        comand_Change_Hero_Face = (Event_comand_Change_Hero_Face *)comand;
        comand_Change_Hero_Face->show();
        break;
    case Change_Vehicle:// 0xD31A,
        Event_comand_Change_Vehicle * comand_Change_Vehicle;
        comand_Change_Vehicle = (Event_comand_Change_Vehicle *)comand;
        comand_Change_Vehicle->show();
        break;
    case Change_System_BGM:// 0xD324,
        Event_comand_Change_System_BGM * comand_Change_System_BGM;
        comand_Change_System_BGM = (Event_comand_Change_System_BGM *)comand;
        comand_Change_System_BGM->show();
        break;
    case Change_System_SE:// 0xD32E,
        Event_comand_Change_System_SE * comand_Change_System_SE;
        comand_Change_System_SE = (Event_comand_Change_System_SE *)comand;
        comand_Change_System_SE->show();
        break;
    case Change_System_GFX:// 0xD338,
        Event_comand_Change_System_GFX * comand_Change_System_GFX;
        comand_Change_System_GFX= (Event_comand_Change_System_GFX *)comand;
        comand_Change_System_GFX->show();
        break;
    case Change_Transition:// 0xD342,
        Event_comand_Change_Transition * comand_Change_Transition;
        comand_Change_Transition = (Event_comand_Change_Transition *)comand;
        comand_Change_Transition->show();
        break;
    case Start_Combat:// 0xD356,
        Event_comand_Start_Combat * comand_Start_Combat;
        comand_Start_Combat = (Event_comand_Start_Combat*)comand;
        comand_Start_Combat->show();
        break;

    case Call_Shop:// 0xD360,
        Event_comand_Call_Shop * comand_Call_Shop;
        comand_Call_Shop = (Event_comand_Call_Shop*)comand;
        comand_Call_Shop->show();
        break;
    case Call_Inn:// 0xD36A,
        Event_comand_Call_Inn * comand_Call_Inn;
        comand_Call_Inn = (Event_comand_Call_Inn*)comand;
        comand_Call_Inn->show();
        break;
    case Enter_hero_name:// 0xD374,
        Event_comand_Enter_hero_name * comand_Enter_hero_name;
        comand_Enter_hero_name =(Event_comand_Enter_hero_name *)comand;
        comand_Enter_hero_name->show();
        break;
    case Store_hero_location:// 0xD444,
        Event_comand_Store_hero_location * comand_Store_hero_location;
        comand_Store_hero_location=(Event_comand_Store_hero_location *)comand;
        comand_Store_hero_location->show();
        break;
    case Recall_to_location:// 0xD44E,
        Event_comand_Recall_to_location * comand_Recall_to_location;
        comand_Recall_to_location=(Event_comand_Recall_to_location *)comand;
        comand_Recall_to_location->show();
        break;
    case Teleport_Vehicle:// 0xD462,
        Event_comand_Teleport_Vehicle * comand_Teleport_Vehicle;
        comand_Teleport_Vehicle=(Event_comand_Teleport_Vehicle *)comand;
        comand_Teleport_Vehicle->show();
        break;
    case Teleport_Event:// 0xD46C,
        Event_comand_Teleport_Event * comand_Teleport_Event;
        comand_Teleport_Event=(Event_comand_Teleport_Event *)comand;
        comand_Teleport_Event->show();
        break;
    case Swap_Event_Positions:// 0xD476,
        Event_comand_Swap_Event_Positions * comand_Swap_Event_Positions;
        comand_Swap_Event_Positions=(Event_comand_Swap_Event_Positions *)comand;
        comand_Swap_Event_Positions->show();
        break;
    case Get_Terrain_ID:// 0xD51E,
        Event_comand_Get_Terrain_ID * comand_Get_Terrain_ID;
        comand_Get_Terrain_ID=(Event_comand_Get_Terrain_ID *)comand;
        comand_Get_Terrain_ID->show();
        break;
    case Get_Event_ID:// 0xD528,
        Event_comand_Get_Event_ID * comand_Get_Event_ID;
        comand_Get_Event_ID=(Event_comand_Get_Event_ID *)comand;
        comand_Get_Event_ID->show();
        break;
    case Erase_screen:// 0xD602,
        Event_comand_Erase_screen * comand_Erase_screen;
        comand_Erase_screen=(Event_comand_Erase_screen *)comand;
        comand_Erase_screen->show();
        break;
    case Show_screen:// 0xD60C,
        Event_comand_Show_screen * comand_Show_screen;
        comand_Show_screen=(Event_comand_Show_screen *)comand;
        comand_Show_screen->show();
        break;
    case Set_screen_tone:// 0xD616,
        Event_comand_Set_screen_tone * comand_Set_screen_tone;
        comand_Set_screen_tone=(Event_comand_Set_screen_tone *)comand;
        comand_Set_screen_tone->show();
        break;
    case Flash_screen:// 0xD620,
        Event_comand_Flash_screen * comand_Flash_screen;
        comand_Flash_screen=(Event_comand_Flash_screen *)comand;
        comand_Flash_screen->show();
        break;
    case Shake_screen:// 0xD62A,
        Event_comand_Shake_screen * comand_Shake_screen;
        comand_Shake_screen=(Event_comand_Shake_screen *)comand;
        comand_Shake_screen->show();
        break;
    case Pan_screen:// 0xD634,
        Event_comand_Pan_screen * comand_Pan_screen;
        comand_Pan_screen=(Event_comand_Pan_screen *)comand;
        comand_Pan_screen->show();
        break;
    case Weather_Effects:// 0xD63E,
        Event_comand_Weather_Effects * comand_Weather_Effects;
        comand_Weather_Effects=(Event_comand_Weather_Effects *)comand;
        comand_Weather_Effects->show();
        break;
    case Show_Picture:// 0xD666,
        Event_comand_Show_Picture * comand_Show_Picture;
        comand_Show_Picture=(Event_comand_Show_Picture *)comand;
        comand_Show_Picture->show();
        break;
    case Move_Picture:// 0xD670,
        Event_comand_Move_Picture * comand_Move_Picture;
        comand_Move_Picture=(Event_comand_Move_Picture *)comand;
        comand_Move_Picture->show();
        break;
    case Erase_Picture:// 0xD67A,
        Event_comand_Erase_Picture * comand_Erase_Picture;
        comand_Erase_Picture=(Event_comand_Erase_Picture *)comand;
        comand_Erase_Picture->show();
        break;
    case Show_Battle_Anim :// 0xD74A,
        Event_comand_Show_Battle_Anim * comand_Show_Battle_Anim;
        comand_Show_Battle_Anim=(Event_comand_Show_Battle_Anim *)comand;
        comand_Show_Battle_Anim->show();
        break;
    case Set_hero_opacity:// 0xD82E,
        Event_comand_Set_hero_opacity * comand_Set_hero_opacity;
        comand_Set_hero_opacity=(Event_comand_Set_hero_opacity *)comand;
        comand_Set_hero_opacity->show();
        break;
    case Flash_event:// 0xD838,
        Event_comand_Flash_event * comand_Flash_event;
        comand_Flash_event=(Event_comand_Flash_event *)comand;
        comand_Flash_event->show();
        break;
    case Move_event:// 0xD842,
        Event_comand_Move_event * comand_Move_event;
        comand_Move_event=(Event_comand_Move_event *)comand;
        comand_Move_event->show();
        break;
    case Wait:// 0xD912,
        Event_comand_Wait * comand_Wait;
        comand_Wait=(Event_comand_Wait *)comand;
        comand_Wait->show();
        break;
    case Play_BGM:// 0xD976,
        Event_comand_Play_BGM * comand_Play_BGM;
        comand_Play_BGM=(Event_comand_Play_BGM *)comand;
        comand_Play_BGM->show();
        break;
    case Fade_out_BGM:// 0xDA00,
        Event_comand_Fade_out_BGM * comand_Fade_out_BGM;
        comand_Fade_out_BGM=(Event_comand_Fade_out_BGM *)comand;
        comand_Fade_out_BGM->show();
        break;
    case Play_sound_effect:// 0xDA1E,

        Event_comand_Play_SE * comand_Play_SE;
        comand_Play_SE=(Event_comand_Play_SE *)comand;
        comand_Play_SE->show();
        break;
    case Play_movie:// 0xDA28,
        Event_comand_Play_movie * comand_Play_movie;
        comand_Play_movie=(Event_comand_Play_movie *)comand;
        comand_Play_movie->show();
        break;
    case Key_input:// 0xDA5A,
        Event_comand_Key_input * comand_Key_input;
        comand_Key_input=(Event_comand_Key_input *)comand;
        comand_Key_input->show();
        break;
    case Change_tile_set:// 0xDB3E	,
        Event_comand_Change_tile * comand_Change_tile;
        comand_Change_tile=(Event_comand_Change_tile *)comand;
        comand_Change_tile->show();
        break;
    case Change_background:// 0xDB48,
        Event_comand_Change_background * comand_Change_background;
        comand_Change_background=(Event_comand_Change_background *)comand;
        comand_Change_background->show();
        break;
    case Change_single_tile:// 0xDB66,
        Event_comand_Change_single_tile * comand_Change_single_tile;
        comand_Change_single_tile=(Event_comand_Change_single_tile*)comand;
        comand_Change_single_tile->show();
        break;
    case Set_teleport_location:// 0xDC22,
        Event_comand_Set_teleport_location * comand_Set_teleport_location;
        comand_Set_teleport_location = (Event_comand_Set_teleport_location*)comand;
        comand_Set_teleport_location->show();
        break;
    case Set_escape_location:// 0xDC36,
        Event_comand_Set_escape_location * comand_Set_escape_location;
        comand_Set_escape_location=(Event_comand_Set_escape_location *)comand;
        comand_Set_escape_location->show();
        break;
    case Enable_system_menu:
        Event_comand_Enable_system_menu * comand_Enable_system_menu;
        comand_Enable_system_menu= (Event_comand_Enable_system_menu *) comand;
        comand_Enable_system_menu->show();
        break;
    case Conditional:// 0xDD6A,
        Event_comand_Conditional * comand_Conditional;
        comand_Conditional= (Event_comand_Conditional *)comand;
        comand_Conditional->show();
        break;
    case Label:// 0xDE4E,
        Event_comand_Label * comand_Label;
        comand_Label = (Event_comand_Label *)comand;
        comand_Label->show();
        break;
    case Go_to_label:// 0xDE58,
        Event_comand_Go_to_label * comand_Go_to_label;
        comand_Go_to_label = (Event_comand_Go_to_label *)comand;
        comand_Go_to_label->show();
        break;
    case Call_event:// 0xE02A,
        Event_comand_Call_event * comand_Call_event;
        comand_Call_event= (Event_comand_Call_event *)comand;
        comand_Call_event->show();
        break;


    }

        //printf("\n Comand id %d ",(*vcEvent_comand[j]).Comand);
      }

    }
    void stEventMap::clear()
    {
        EventName="";
        X_position=0;
        Y_position=0;
        vcPage.clear();
    }
    void stEventMap::show()
    { int i;
    printf("\nEventName  %s\n", EventName.c_str());
    printf("\nX_position %d ",X_position);
    printf("\nY_position %d ",Y_position);
    i=vcPage.size();
    for (int j=0; j<i;j++)
      {
	printf("\n \nvcPage %d \n",j+1);
        vcPage[j].show();
       }

    }
