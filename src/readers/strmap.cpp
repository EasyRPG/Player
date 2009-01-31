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

      for (int j=0; j<i;j++)
      {   //printf("\n \nComand num %d \n",j+1);
        //  vcEvent_comand[j].show();
        printf("\n Comand id %d ",(*vcEvent_comand[j]).Comand);
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
