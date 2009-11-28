    #include <cstdlib>
    #include <cstdio>
    #include <string>
    #include "tools.h"
    #include "stevent.h"
   void Event_comand_Simple:: show(){
   printf("\n Comand %d ",Comand);
   }
 void Event_comand:: show(){
   printf("\n Comand %d ",Comand);
   }

  void Event_comand_Message:: show(){
  printf("\nText  %s\n", Text.c_str());
  }
  void Event_comand_Message_options:: show(){
   printf("\nTransparency %d ",Transparency);
   printf("\nPosition %d ",Position);
   printf("\nPrevent_Hiding %d ",Prevent_Hiding);
   printf("\nAllow_parallel %d ",Allow_parallel);
   }
  void Event_comand_Select_face:: show(){
   printf("\nFilename  %s\n", Filename.c_str());
   printf("\nIndex %d ",Index);
   printf("\nPlace_at_Right %d ",Place_at_Right);
   printf("\nFlip_Image %d ",Flip_Image);
   }
  void Event_comand_Show_choice:: show(){
    printf("\nText  %s\n", Text.c_str());
   printf("\nCancel_option %d ",Cancel_option);
   }
  void Event_comand_Show_choice_option:: show(){

   printf("\nText  %s\n", Text.c_str());
   printf("\nChoice_number %d ",Choice_number);
   }
   void Event_comand_Number_input:: show(){
   printf("\nPrevent_Hiding %d ",Digits_to_input);
   printf("\nAllow_parallel %d ",variable_to_store);
   }
   void Event_comand_Change_switch:: show(){
   printf("\nMode %d ",Mode);
   printf("\nstart_switch %d ",start_switch);
   printf("\nend_switch %d ",end_switch);
   printf("\ntoggle_option %d ",toggle_option);
   }

  void Event_comand_Change_var:: show(){
   printf("\nMode %d ",Mode);
   printf("\nstart_switch %d ",start_switch);
   printf("\nend_switch %d ",end_switch);
   printf("\noperation %d ",operation);
   printf("\nop_mode %d ",op_mode);
   printf("\nop_data1 %d ",op_data1);
   printf("\nop_data2 %d ",op_data2);
   }
   void Event_comand_Timer_manipulation:: show(){
   printf("\nSet %d ",Set);
   printf("\nBy_Value %d ",By_Value);
   printf("\nSeconds %d ",Seconds);
   printf("\nDisplay %d ",Display);
   printf("\nRuns_in_Battle %d ",Runs_in_Battle);
   printf("\nop_data1 %d ",op_data1);
   }
   void Event_comand_Change_cash_held:: show(){
   printf("\nAdd %d ",Add);
   printf("\nBy_Value %d ",By_Value);
   printf("\nAmount %d ",Amount);
   }
   void Event_comand_Change_inventory:: show(){
   printf("\nAdd %d ",Add);
   printf("\nBy_ID %d ",By_ID);
   printf("\nItem_ID %d ",Item_ID);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   }
   void Event_comand_Change_party:: show(){
   printf("\nAdd %d ",Add);
   printf("\nBy_ID %d ",By_ID);
   printf("\nHero_ID %d ",Hero_ID);
   }
   void Event_comand_Change_experience:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nAdd %d ",Add);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   printf("\nShow_Level_Up_Message %d ",Show_Level_Up_Message);
   }
   void Event_comand_Change_level:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nAdd %d ",Add);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   printf("\nShow_Level_Up_Message %d ",Show_Level_Up_Message);
   }
   void Event_comand_Change_statistics:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nAdd %d ",Add);
   printf("\nStat %d ",Stat);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   }
   void Event_comand_Learn_forget_skill:: show(){
   printf("\nHero %d ",Hero);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nLearn %d ",Learn);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   }
   void Event_comand_Change_equipment:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nAdd_Remove %d ",Add_Remove);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   }
   void Event_comand_Change_HP:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nAdd %d ",Add);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   printf("\nPossible_Death %d ",Possible_Death);
   }
   void Event_comand_Change_MP:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nAdd %d ",Add);
   printf("\nBy_Count %d ",By_Count);
   printf("\nCount %d ",Count);
   }
   void Event_comand_Change_Status:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nInflict %d ",Inflict);
   printf("\nStatus_effect %d ",Status_effect);
   }
   void Event_comand_Full_Recovery:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   }
   void Event_comand_Inflict_Damage:: show(){
   printf("\nAll %d ",All);
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nInflict %d ",Damage);
   printf("\nDefense_effect %d ",Defense_effect);
   printf("\nMind_effect %d ",Mind_effect);
   printf("\nVariance %d ",Variance);
   printf("\nSave_damage_to_var %d ",Save_damage_to_var);
   printf("\nVar_ID %d ",Var_ID);
   }
   void Event_comand_Change_Hero_Name:: show(){
    printf("\nNew_name  %s\n", New_name.c_str());
    printf("\nHero_ID %d ",Hero_ID);
   }
   void Event_comand_Change_Hero_Class:: show(){
    printf("\n New_class  %s\n", strNew_class.c_str());
   printf("\n Hero_ID %d ",Hero_ID);
   }

   void Event_comand_Change_Hero_Graphic:: show(){
    printf("\nNew_graphic  %s\n", New_graphic.c_str());
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nSprite_ID %d ",Sprite_ID);
   printf("\nTransparent %d ",Transparent);
   }
   void Event_comand_Change_Hero_Face:: show(){
   printf("\nNew_graphic  %s\n", New_graphic.c_str());
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nFace_ID %d ",Face_ID);
   }
   void Event_comand_Change_Vehicle:: show(){
   printf("\nNew_graphic  %s\n", New_graphic.c_str());
   printf("\nVehicle_ID %d ",Vehicle_ID);
   printf("\nSprite_ID %d ",Sprite_ID);
   }
   void Event_comand_Change_System_BGM:: show(){
   printf("\nNew_BGM  %s\n", New_BGM.c_str());
   printf("\nBGM_ID %d ",BGM_ID);
   printf("\nFadein_time %d ",Fadein_time);
   printf("\nVolume %d ",Volume);
   printf("\nTempo %d ",Tempo);
   printf("\nBalance %d ",Balance);
   }
   void Event_comand_Change_System_SE:: show(){
   printf("\nNew_SE  %s\n", New_SE.c_str());
   printf("\nSE_ID %d ",SE_ID);
   printf("\nVolume %d ",Volume);
   printf("\nTempo %d ",Tempo);
   printf("\nBalance %d ",Balance);
   }
   void Event_comand_Change_System_GFX:: show(){
   printf("\nNew_graphic  %s\n", New_graphic.c_str());
   printf("\nStretch %d ",Stretch);
   printf("\nGothic_font %d ",Gothic_font);
   }
   void Event_comand_Change_Transition:: show(){
   printf("\nStyle %d ",Type);
   printf("\nStyle %d ",Transition);
   }

    void Event_comand_Start_Combat:: show(){
   printf("\nBackground  %s\n", Background.c_str());
   printf("\nFixed_group %d ",Fixed_group);
   printf("\nGroup_ID %d ",Group_ID);
   printf("\nBackground_Flag %d ",Background_Flag);
   printf("\nEscape %d ",Escape);
   printf("\nDefeat %d ",Defeat);
   printf("\nFirst_strike %d ",First_strike);

   }
    void Event_comand_Call_Shop:: show(){
   printf("\nStyle %d ",Style);
   printf("\nMessage_style %d ",Message_style);
   printf("\nHandler_on_purchase %d ",Handler_on_purchase);
   std:: vector <int> Item_IDs;
   }
    void Event_comand_Call_Inn:: show(){
   printf("\nStyle %d ",Style);
   printf("\nMessage_style %d ",Message_style);
   printf("\nCost %d ",Cost);
   printf("\nHandler_on_rest %d ",Handler_on_rest);
   }
    void Event_comand_Enter_hero_name:: show(){
   printf("\nHero_ID %d ",Hero_ID);
   printf("\nInitial_method %d ",Initial_method);
   printf("\nShow_initial_name %d ",Show_initial_name);
   }
    void Event_comand_Teleport_Party:: show(){
   printf("\nMap_ID %d ",Map_ID);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);
   }
   void Event_comand_Store_hero_location:: show(){
   printf("\nMap_ID_Var %d ",Map_ID_Var);
   printf("\nX_Var %d ",X_Var);
   printf("\nY_Var %d ",Y_Var);
   }
   void Event_comand_Recall_to_location:: show(){
   printf("\nMap_ID_Var %d ",Map_ID_Var);
   printf("\nX_Var %d ",X_Var);
   printf("\nY_Var %d ",Y_Var);
   }

   void Event_comand_Teleport_Vehicle:: show(){
   printf("\ntype %d ",type);
   printf("\nLocation %d ",Location);
   printf("\nMap_ID %d ",Map_ID);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);
   }
   void Event_comand_Teleport_Event:: show(){
   printf("\nEvent_ID %d ",Event_ID);
   printf("\nBy_value %d ",By_value);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);
   }
   void Event_comand_Swap_Event_Positions:: show(){
   printf("\nFirst_event %d ",First_event);
   printf("\nSecond_event %d ",Second_event);
   }
   void Event_comand_Get_Terrain_ID:: show(){
   printf("\nBy_value %d ",By_value);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);
   printf("\nVariable_to_store %d ",Variable_to_store);
   }
   void Event_comand_Get_Event_ID:: show(){
   printf("\nBy_value %d ",By_value);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);
   printf("\nVariable_to_store %d ",Variable_to_store);
   }
   void Event_comand_Erase_screen:: show(){
   printf("\nTransition %d ",Transition);
   }
   void Event_comand_Show_screen:: show(){
   printf("\nShow_screen %d ",Show_screen);
   }
   void Event_comand_Set_screen_tone:: show(){
   printf("\nRed_diffuse %d ",Red_diffuse);
   printf("\nGreen_diffuse %d ",Green_diffuse);
   printf("\nBlue_diffuse %d ",Blue_diffuse);
   printf("\nChroma_diffuse %d ",Chroma_diffuse);
   printf("\nLength %d ",Length);
   printf("\nWait %d ",Wait);
   }
   void Event_comand_Flash_screen:: show(){
   printf("\nRed_diffuse %d ",Red_diffuse);
   printf("\nGreen_diffuse %d ",Green_diffuse);
   printf("\nBlue_diffuse %d ",Blue_diffuse);
   printf("\nStrength %d ",Strength);
   printf("\nLength %d ",Length);
   printf("\nWait %d ",Wait);
   }
   void Event_comand_Shake_screen:: show(){
   printf("\nPower %d ",Power);
   printf("\nSpeed %d ",Speed);
   printf("\nLength %d ",Length);
   printf("\nWait %d ",Wait);
   }
   void Event_comand_Pan_screen:: show(){
   printf("\nType %d ",Type);
   printf("\nDirection %d ",Direction);
   printf("\nDistance %d ",Distance);
   printf("\nSpeed %d ",Speed);
   printf("\nWait %d ",Wait);
   }
   void Event_comand_Weather_Effects:: show(){
   printf("\nType %d ",Type);
   printf("\nSpeed %d ",Speed);
   }
   void Event_comand_Show_Picture:: show(){

   printf("\nImage_file  %s\n", Image_file.c_str());
   printf("\nPicture_ID %d ",Picture_ID);
   printf("\nBy_Value %d ",By_Value);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);
   printf("\nMove_Map %d ",Move_Map);
   printf("\nMagnification %d ",Magnification);
   printf("\nOpacity %d ",Opacity);
   printf("\nUse_color_key %d ",Use_color_key);
   printf("\nRed_diffuse %d ",Red_diffuse);
   printf("\nGreen_diffuse %d ",Green_diffuse);
   printf("\nBlue_diffuse %d ",Blue_diffuse);
   printf("\nChroma_diffuse %d ",Chroma_diffuse);
   printf("\nEffect %d ",Effect);
   printf("\nPower %d ",Power);
   }
   void Event_comand_Move_Picture:: show(){

   printf("\nPicture_ID %d ",Picture_ID);
   printf("\nBy_Value %d ",By_Value);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);

   printf("\nMagnification %d ",Magnification);
   printf("\nOpacity %d ",Opacity);
   printf("\nRed_diffuse %d ",Red_diffuse);
   printf("\nGreen_diffuse %d ",Green_diffuse);
   printf("\nBlue_diffuse %d ",Blue_diffuse);
   printf("\nChroma_diffuse %d ",Chroma_diffuse);

   printf("\nEffect %d ",Effect);
   printf("\nPower %d ",Power);
   printf("\nLength %d ",Length);
   printf("\nWait %d ",Wait);

   }
   void Event_comand_Erase_Picture:: show(){
   printf("\nPicture_ID %d ",Picture_ID);
   }
   void Event_comand_Show_Battle_Anim:: show(){
   printf("\nAnimation_ID %d ",Animation_ID);
   printf("\nTarget %d ",Target);
   printf("\nWait %d ",Wait);
   printf("\nFull_screen %d ",Full_screen);
   }
   void Event_comand_Set_hero_opacity:: show(){
   printf("\nOpacity %d ",Opacity);
   }
   void Event_comand_Flash_event:: show(){
   printf("\nTarget %d ",Target);
   printf("\nRed %d ",Red);
   printf("\nGreen %d ",Green);
   printf("\nBlue %d ",Blue);
   printf("\nStrength %d ",Strength);
   printf("\nLength %d ",Length);
   printf("\nWait %d ",Wait);
   }
   void Event_comand_Move_event:: show(){
   int j,i;
   printf("\nTarget %d ",Target);
   printf("\nFrequency %d ",Frequency);
   printf("\nRepeat_actions %d ",Repeat_actions);
   printf("\nIgnore_impossible %d ",Ignore_impossible);
  i=comand_moves.size();
  for ( j=0; j<i;j++)
  printf("\nDirections, %d %d ",j,comand_moves[j]->Comand);
   }
   void Event_comand_Wait:: show(){
    printf("\nLength %d ",Length);
   }
   void Event_comand_Play_BGM:: show(){

   printf("\nBGM_name  %s\n", BGM_name.c_str());
   printf("\nFade_in_time %d ",Fade_in_time);
   printf("\nVolume %d ",Volume);
   printf("\nTempo %d ",Tempo);
   printf("\nBalance %d ",Balance);
   }
   void Event_comand_Fade_out_BGM:: show(){
   printf("\nFade_in_time %d ",Fade_in_time);
   }
   void Event_comand_Play_SE:: show(){
   printf("\nSE_name  %s\n", SE_name.c_str());
   printf("\nVolume %d ",Volume);
   printf("\nTempo %d ",Tempo);
   printf("\nBalance %d ",Balance);
   }
   void Event_comand_Play_movie:: show(){
   printf("\nMovie_file  %s\n", Movie_file.c_str());
   printf("\nBy_value %d ",By_value);
   printf("\nX %d ",X);
   printf("\nY %d ",Y);
   printf("\nWidth %d ",Width);
   printf("\nHeight %d ",Height);
   }

   void Event_comand_Key_input:: show(){
   printf("\nVariable_to_store %d ",Variable_to_store);
   printf("\nWait_for_key %d ",Wait_for_key);
   printf("\nDirectionals %d ",Directionals);
   printf("\nAccept %d ",Accept);
   printf("\nCancel %d ",Cancel);
   }
   void Event_comand_Change_tile:: show(){
   printf("\nNew_tile %d ",New_tile);
   }
   void Event_comand_Change_background:: show(){
   printf("\nParallax_BG  %s\n", Parallax_BG.c_str());
   printf("\nX_pan %d ",X_pan);
   printf("\nY_pan %d ",Y_pan);
   printf("\nX_auto_pan %d ",X_auto_pan);
   printf("\nX_pan_speed %d ",X_pan_speed);
   printf("\nY_auto_pan %d ",Y_auto_pan);
   printf("\nY_pan_speed %d ",Y_pan_speed);
   }
 void Event_comand_Change_encounter_rate:: show(){
       printf("\nEncounter_rate %d ",Encounter_rate);
   }
 void Event_comand_Change_single_tile:: show(){
       printf("\nLayer %d ",Layer);
       printf("\nOld_tile %d ",Old_tile);
       printf("\nNew_tile %d ",New_tile);
   }
 void Event_comand_Set_teleport_location:: show(){
       printf("\nAdd %d ",Add);
       printf("\nMap_ID %d ",Map_ID);
       printf("\nX %d ",X);
       printf("\nY %d ",Y);
       printf("\nSwitch %d ",Switch);
       printf("\nSwitch_ID %d ",Switch_ID);
   }

 void Event_comand_Enable_teleport:: show(){
       printf("\nEnable %d ",Enable);
   }

 void Event_comand_Set_escape_location:: show(){
       printf("\nMap_ID %d ",Map_ID);
       printf("\nX %d ",X);
       printf("\nY %d ",Y);
       printf("\nSwitch %d ",Switch);
       printf("\nSwitch_ID %d ",Switch_ID);
   }
 void Event_comand_Enable_escape:: show(){
       printf("\nEnable %d ",Enable);
   }
 void Event_comand_Enable_saving:: show(){
      printf("\nEnable %d ",Enable);
   }
 void Event_comand_Enable_system_menu:: show(){
      printf("\nEnable %d ",Enable);
   }
 void Event_comand_Conditional:: show(){
    // por confirmar
    printf("\nName %s ",Name_data.c_str());// type of operation

    printf("\ntype_of_conditional %d ",type_of_conditional);// type of operation
    printf("\nID %d ",ID);//more lique a second data
    printf("\nOp_code %d ",Op_code);// 0 for active 1  for unactive onthe fases
    printf("\nID_2 %d ",ID_2);// id for de var
    printf("\nCount %d ",Count);
    printf("\nExeption %d ",Exeption);
   }
 void Event_comand_Label:: show(){
  printf("\nLabel_id %d ",Label_id);
   }
 void Event_comand_Go_to_label:: show(){
  printf("\nLabel_id %d ",Label_id);
 }
 void Event_comand_Call_event:: show(){
  printf("\nMethod %d ",Method);
  printf("\nEvent_ID %d ",Event_ID);
  printf("\nEvent_page %d ",Event_page);
   }
  void Event_comand_Comment_Text:: show(){
  printf("\nText  %s\n", Text.c_str());//0x15
  }
  void Event_comand_Add_line_to_comment:: show(){
  printf("\nText  %s\n", Text.c_str());//0x15
  }

  void Event_comand_Change_Profetion:: show(){
  printf("\nHero_ID %d ",Hero_ID);
  printf("\nComands %d ",Comands);
  printf("\nLevels %d ",Levels);
  printf("\nSkills %d ",Skills);
  printf("\nValues %d ",Values);
  printf("\nOptions %d ",Options);
 }
   void Event_comand_Change_Batle_Comands:: show() {
  printf("\n remove %d ",remove);
  printf("\n Hero_ID %d ",Hero_ID);
  printf("\n Batle_Comand %d ",Batle_Comand);
  printf("\n Add %d ",Add);
 }

Event_comand * stEvent::EventcommandMessageChunk(int Command,int Depth,FILE * Stream)
{
    Event_comand_Message * Message;
    Message = new Event_comand_Message();
    Message->Comand=Command;
    Message->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream); //longitud de cadena
    Message->Text = ReadString(Stream, ChunkInfo.Length);
    ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 00
    return (Message);
}
Event_comand * stEvent::EventcommandMessageoptionsChunk(int Command,int Depth,FILE * Stream)
{
   Event_comand_Message_options * Message_options;
   Message_options= new Event_comand_Message_options();
   Message_options->Comand=Command;
   Message_options->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
   ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 04
   Message_options->Transparency= ReadCompressedInteger(Stream);
   Message_options->Position=  ReadCompressedInteger(Stream);
   Message_options->Prevent_Hiding=  ReadCompressedInteger(Stream);
   Message_options->Allow_parallel=  ReadCompressedInteger(Stream);
return (Message_options);
}
Event_comand * stEvent::EventcommandSelectfaceChunk(int Command,int Depth,FILE * Stream)
{
   Event_comand_Select_face * Select_face;
   Select_face = new Event_comand_Select_face();
   Select_face->Comand=Command;
   Select_face->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream); //longitud de cadena
   Select_face->Filename = ReadString(Stream, ChunkInfo.Length);

   ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 00
   Select_face->Index= ReadCompressedInteger(Stream);
   Select_face->Place_at_Right=  ReadCompressedInteger(Stream);
   Select_face->Flip_Image=  ReadCompressedInteger(Stream);
   return (Select_face);
}


Event_comand * stEvent::EventcommandShowchoiceChunk(int Command,int Depth,FILE * Stream)
{
   Event_comand_Show_choice * choice;
   choice = new Event_comand_Show_choice();
   choice->Depth=Depth;
   choice->Comand=Command;
   ChunkInfo.Length= ReadCompressedInteger(Stream); 	//longitud de cadena
   choice->Text = ReadString(Stream, ChunkInfo.Length);	// opcion /opcion/opcion
   ChunkInfo.Length= ReadCompressedInteger(Stream); //longitud 01
   choice->Cancel_option=  ReadCompressedInteger(Stream);
   return (choice);
}
Event_comand * stEvent::EventcommandSimpleEvent_comand(int Command,int Depth,FILE * Stream)
{
  Event_comand_Simple * comand;
  comand = new Event_comand_Simple();
  comand->Comand=Command;
  comand->Depth=Depth;
  ChunkInfo.Length= ReadCompressedInteger(Stream); //longitud 00
  ChunkInfo.Length= ReadCompressedInteger(Stream); //longitud 00
  return (comand);
}
Event_comand * stEvent::EventcommandShow_choice_option(int Command,int Depth,FILE * Stream)
{
  Event_comand_Show_choice_option * comand;
  comand = new Event_comand_Show_choice_option();
  comand->Comand=Command;
  comand->Depth=Depth;
  ChunkInfo.Length= ReadCompressedInteger(Stream); 	//longitud de cadena
  comand->Text = ReadString(Stream, ChunkInfo.Length);	// choice
  ChunkInfo.Length= ReadCompressedInteger(Stream); //longitud 01
  //Choice number (zero offset)
  comand->Choice_number=  ReadCompressedInteger(Stream);
  return (comand);
}
Event_comand * stEvent::EventcommandNumber_input(int Command,int Depth,FILE * Stream)
{
  Event_comand_Number_input * comand;
  comand = new Event_comand_Number_input();
  comand->Comand=Command;
  comand->Depth=Depth;
  ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
  ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 0
  comand->Digits_to_input= ReadCompressedInteger(Stream);
  comand->variable_to_store=  ReadCompressedInteger(Stream);
  return (comand);
}
Event_comand * stEvent::EventcommandChange_switch(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_switch * comand;
  comand = new Event_comand_Change_switch();
  comand->Comand=Command;
  comand->Depth=Depth;
                          ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 04
                         //Mode (single, range, indirect)
  comand->Mode= ReadCompressedInteger(Stream);
  comand->start_switch=  ReadCompressedInteger(Stream);
  comand->end_switch= ReadCompressedInteger(Stream);
  comand->toggle_option=  ReadCompressedInteger(Stream);

  return (comand);
}
Event_comand * stEvent::EventcommandChange_var(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_var * comand;
  comand = new Event_comand_Change_var();
  comand->Comand=Command;
  comand->Depth=Depth;
                             ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 07
                         //Mode (single, range, indirect)
                          comand->Mode= ReadCompressedInteger(Stream);
                          comand->start_switch=  ReadCompressedInteger(Stream);
                          comand->end_switch= ReadCompressedInteger(Stream);
                          comand->operation=  ReadCompressedInteger(Stream);
                          comand->op_mode=  ReadCompressedInteger(Stream);
                          comand->op_data1=  ReadCompressedInteger(Stream);
                          comand->op_data2=  ReadCompressedInteger(Stream);
  return (comand);
}
Event_comand * stEvent::EventcommandTimer_manipulation(int Command,int Depth,FILE * Stream)
{
  Event_comand_Timer_manipulation * comand;
  comand = new Event_comand_Timer_manipulation();
  comand->Comand=Command;
  comand->Depth=Depth;
  ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
  ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 06

                         //Set (1:start, 2:stop)
  comand->Set= ReadCompressedInteger(Stream);
  comand->By_Value=  ReadCompressedInteger(Stream);
  comand->Seconds= ReadCompressedInteger(Stream);
  comand->Display=  ReadCompressedInteger(Stream);
  comand->Runs_in_Battle=  ReadCompressedInteger(Stream);
  if(ChunkInfo.Length<5)
   comand->op_data1=  ReadCompressedInteger(Stream);

  return (comand);
}
 Event_comand * stEvent::EventcommandChange_cash_held(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_cash_held * comand;
  comand = new Event_comand_Change_cash_held();
  comand->Comand=Command;
  comand->Depth=Depth;
                         ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 06
                          comand->Add= ReadCompressedInteger(Stream);
                          comand->By_Value=  ReadCompressedInteger(Stream);
                          comand->Amount= ReadCompressedInteger(Stream);
  return (comand);
}
  Event_comand * stEvent::EventcommandChange_inventory(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_inventory * comand;
  comand = new Event_comand_Change_inventory();
  comand->Comand=Command;
  comand->Depth=Depth;
                           ChunkInfo.Length= ReadCompressedInteger(Stream);// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 06
                          comand->Add= ReadCompressedInteger(Stream);
                          comand->By_ID=  ReadCompressedInteger(Stream);
                          comand->Item_ID= ReadCompressedInteger(Stream);
                          comand->By_Count=  ReadCompressedInteger(Stream);
                          comand->Count=  ReadCompressedInteger(Stream);

   return (comand);
}
  Event_comand * stEvent::EventcommandChange_party(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_party * comand;
  comand = new Event_comand_Change_party();
  comand->Comand=Command;
  comand->Depth=Depth;
                           ChunkInfo.Length= ReadCompressedInteger(Stream);// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 06
                         //Add (1:remove)
                          comand->Add= ReadCompressedInteger(Stream);
                          comand->By_ID=  ReadCompressedInteger(Stream);
                          comand->Hero_ID= ReadCompressedInteger(Stream);
   return (comand);
}
  Event_comand * stEvent::EventcommandChange_experience(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_experience * comand; //aqui
  comand = new Event_comand_Change_experience();
  comand->Comand=Command;
  comand->Depth=Depth;
                          ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 07
                          comand->All= ReadCompressedInteger(Stream);
                          comand->Hero_ID=  ReadCompressedInteger(Stream);
                          comand->Add= ReadCompressedInteger(Stream);
                          comand->By_Count=  ReadCompressedInteger(Stream);
                          comand->Count=  ReadCompressedInteger(Stream);
                          comand->Show_Level_Up_Message=  ReadCompressedInteger(Stream);
   return (comand);
}
  Event_comand * stEvent::EventcommandChange_level(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_level * comand;
  comand = new Event_comand_Change_level();
comand->Comand=Command;
  comand->Depth=Depth;
                         ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 07
                          comand->All= ReadCompressedInteger(Stream);
                          comand->Hero_ID=  ReadCompressedInteger(Stream);
                          comand->Add= ReadCompressedInteger(Stream);
                          comand->By_Count=  ReadCompressedInteger(Stream);
                          comand->Count=  ReadCompressedInteger(Stream);
                          comand->Show_Level_Up_Message=  ReadCompressedInteger(Stream);
   return (comand);
}
  Event_comand * stEvent::EventcommandChange_statistics(int Command,int Depth,FILE * Stream)
{
  Event_comand_Change_statistics * comand;
  comand = new Event_comand_Change_statistics();
  comand->Comand=Command;comand->Depth=Depth;
                          ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 07
                          comand->All= ReadCompressedInteger(Stream);
                          comand->Hero_ID=  ReadCompressedInteger(Stream);
                          comand->Add= ReadCompressedInteger(Stream);
                          comand->Stat=  ReadCompressedInteger(Stream);
                          comand->By_Count=  ReadCompressedInteger(Stream);
                          comand->Count=  ReadCompressedInteger(Stream);
   return (comand);
}
   Event_comand * stEvent::EventcommandLearn_forget_skill(int Command,int Depth,FILE * Stream)
{
  Event_comand_Learn_forget_skill * comand;
  comand = new Event_comand_Learn_forget_skill();
  comand->Comand=Command;
  comand->Depth=Depth;
                          ChunkInfo.Length= ReadCompressedInteger(Stream); 	// longitud 00
                           ChunkInfo.Length= ReadCompressedInteger(Stream); // longitud 07
                          comand->Hero= ReadCompressedInteger(Stream);
                          comand->Hero_ID=  ReadCompressedInteger(Stream);
                          comand->Learn= ReadCompressedInteger(Stream);
                          comand->By_Count=  ReadCompressedInteger(Stream);
                          comand->Count=  ReadCompressedInteger(Stream);
   return (comand);
   }

Event_comand * stEvent::EventcommandChange_equipment(int Command,int Depth,FILE * Stream) {
  Event_comand_Change_equipment * comand;
  comand = new Event_comand_Change_equipment();
    comand->Comand=Command;
    comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->All=ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
    comand->Add_Remove=ReadCompressedInteger(Stream);
    comand->By_Count=ReadCompressedInteger(Stream);
    comand->Count=ReadCompressedInteger(Stream);
      return (comand);
}
Event_comand * stEvent::EventcommandChange_HP(int Command,int Depth,FILE * Stream) {
    Event_comand_Change_HP * comand;
  comand = new Event_comand_Change_HP();
    comand->Comand=Command;
    comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->All=ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
    comand->Add=ReadCompressedInteger(Stream);
    comand->By_Count=ReadCompressedInteger(Stream);
    comand->Count=ReadCompressedInteger(Stream);
    comand->Possible_Death=ReadCompressedInteger(Stream);
      return (comand); }
Event_comand * stEvent::EventcommandChange_MP(int Command,int Depth,FILE * Stream) {
    Event_comand_Change_MP * comand;
  comand = new Event_comand_Change_MP();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->All=ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
    comand->Add=ReadCompressedInteger(Stream);
    comand->By_Count=ReadCompressedInteger(Stream);
    comand->Count=ReadCompressedInteger(Stream);
      return (comand); }
Event_comand * stEvent::EventcommandChange_Status(int Command,int Depth,FILE * Stream) {
    Event_comand_Change_Status * comand;
  comand = new Event_comand_Change_Status();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->All=ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
    comand->Inflict=ReadCompressedInteger(Stream);
    comand->Status_effect=ReadCompressedInteger(Stream);
      return (comand); }
Event_comand * stEvent::EventcommandFull_Recovery(int Command,int Depth,FILE * Stream) {
    Event_comand_Full_Recovery * comand;
  comand = new Event_comand_Full_Recovery();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->All=ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
      return (comand); }
Event_comand * stEvent::EventcommandInflict_Damage(int Command,int Depth,FILE * Stream) {
    Event_comand_Inflict_Damage * comand;
  comand = new Event_comand_Inflict_Damage();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->All=ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
    comand->Damage=ReadCompressedInteger(Stream);
    comand->Defense_effect=ReadCompressedInteger(Stream);
    comand->Mind_effect=ReadCompressedInteger(Stream);
    comand->Variance=ReadCompressedInteger(Stream);
    comand->Save_damage_to_var=ReadCompressedInteger(Stream);
    comand->Var_ID=ReadCompressedInteger(Stream);
    return (comand);
    }

Event_comand * stEvent::EventcommandChange_Hero_Name(int Command,int Depth,FILE * Stream) {
    Event_comand_Change_Hero_Name * comand;
  comand = new Event_comand_Change_Hero_Name();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->New_name= ReadString(Stream, ChunkInfo.Length);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
      return (comand);
      }
Event_comand * stEvent::EventcommandChange_Hero_Class(int Command,int Depth,FILE * Stream) {
    Event_comand_Change_Hero_Class * comand;
  comand = new Event_comand_Change_Hero_Class();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->strNew_class= ReadString(Stream, ChunkInfo.Length);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Hero_ID= ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandChange_Hero_Graphic(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_Hero_Graphic * comand;
  comand = new Event_comand_Change_Hero_Graphic();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->New_graphic= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Hero_ID=ReadCompressedInteger(Stream);
   comand->Sprite_ID=ReadCompressedInteger(Stream);
   comand->Transparent=ReadCompressedInteger(Stream);
   return (comand);
  }

Event_comand * stEvent::EventcommandChange_Hero_Face(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_Hero_Face * comand;
  comand = new Event_comand_Change_Hero_Face();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->New_graphic= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Hero_ID=ReadCompressedInteger(Stream);
   comand->Face_ID=ReadCompressedInteger(Stream);
      return (comand);
      }
Event_comand * stEvent::EventcommandChange_Vehicle(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_Vehicle * comand;
  comand = new Event_comand_Change_Vehicle();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->New_graphic= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Vehicle_ID=ReadCompressedInteger(Stream);
   comand->Sprite_ID=ReadCompressedInteger(Stream);
   return (comand);
      }
Event_comand * stEvent::EventcommandChange_System_BGM(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_System_BGM * comand;
  comand = new Event_comand_Change_System_BGM();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->New_BGM= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->BGM_ID=ReadCompressedInteger(Stream);
   comand->Fadein_time=ReadCompressedInteger(Stream);
   comand->Volume=ReadCompressedInteger(Stream);
   comand->Tempo=ReadCompressedInteger(Stream);
   comand->Balance=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandChange_System_SE(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_System_SE * comand;
  comand = new Event_comand_Change_System_SE();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->New_SE= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->SE_ID=ReadCompressedInteger(Stream);
   comand->Volume=ReadCompressedInteger(Stream);
   comand->Tempo=ReadCompressedInteger(Stream);
   comand->Balance=ReadCompressedInteger(Stream);
   return (comand);
      }
Event_comand * stEvent::EventcommandChange_System_GFX(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_System_GFX * comand;
  comand = new Event_comand_Change_System_GFX();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->New_graphic= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Stretch=ReadCompressedInteger(Stream);
   comand->Gothic_font=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandChange_Transition(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_Transition * comand;
  comand = new Event_comand_Change_Transition();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Type=ReadCompressedInteger(Stream);
   comand->Transition=ReadCompressedInteger(Stream);
   return (comand);
   }

Event_comand * stEvent::EventcommandStart_Combat(int Command,int Depth,FILE * Stream) {
   Event_comand_Start_Combat * comand;
  comand = new Event_comand_Start_Combat();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Background= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);  //9 datos en el 2003
    //6 datos en el 2000
   comand->Fixed_group=ReadCompressedInteger(Stream);
   comand->Group_ID=ReadCompressedInteger(Stream);
   comand->Background_Flag=ReadCompressedInteger(Stream);
   comand->Escape=ReadCompressedInteger(Stream);
   comand->Defeat=ReadCompressedInteger(Stream);
   comand->First_strike=ReadCompressedInteger(Stream);
   if(ChunkInfo.Length==9)//datos que desconosco
   {
                                  ReadCompressedInteger(Stream);
                                  ReadCompressedInteger(Stream);
                                  ReadCompressedInteger(Stream);
   }
   return (comand);
   }
Event_comand * stEvent::EventcommandCall_Shop(int Command,int Depth,FILE * Stream) {
   Event_comand_Call_Shop * comand;
  comand = new Event_comand_Call_Shop();
   int itemasnum,data;
  string name;
   comand->Comand=Command;comand->Depth=Depth;

   ChunkInfo.Length= ReadCompressedInteger(Stream); //primera longitud
                           name = ReadString(Stream, ChunkInfo.Length);
                          printf("%s", name.c_str());
                            ChunkInfo.Length= ReadCompressedInteger(Stream); //segunda longitud


   comand->Style=ReadCompressedInteger(Stream);
   ChunkInfo.Length--;
   comand->Message_style=ReadCompressedInteger(Stream);
   ChunkInfo.Length--;
   comand->Handler_on_purchase=ReadCompressedInteger(Stream);
   ChunkInfo.Length--;
   itemasnum=ReadCompressedInteger(Stream);//0x00
  ChunkInfo.Length--;

    while(ChunkInfo.Length--)//seguro longitud
    {
        data= ReadCompressedInteger(Stream);
        comand->Item_IDs.push_back(data);
    }

return (comand);
   }
Event_comand * stEvent::EventcommandCall_Inn(int Command,int Depth,FILE * Stream) {
   Event_comand_Call_Inn * comand;
  comand = new Event_comand_Call_Inn();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream); //3 datos 2003
   //4 datos en 2000
  if(ChunkInfo.Length==4)
   comand->Style=ReadCompressedInteger(Stream);

   comand->Message_style=ReadCompressedInteger(Stream);
   comand->Cost=ReadCompressedInteger(Stream);
   comand->Handler_on_rest=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandEnter_hero_name(int Command,int Depth,FILE * Stream) {
   Event_comand_Enter_hero_name * comand;
  comand = new Event_comand_Enter_hero_name();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Hero_ID=ReadCompressedInteger(Stream);
   comand->Initial_method=ReadCompressedInteger(Stream);
   comand->Show_initial_name=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandTeleport_Party(int Command,int Depth,FILE * Stream) {
   Event_comand_Teleport_Party * comand;
   comand = new Event_comand_Teleport_Party();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Map_ID=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);
   if(ChunkInfo.Length==4)
   ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandStore_hero_location(int Command,int Depth,FILE * Stream) {
   Event_comand_Store_hero_location * comand;
   comand = new Event_comand_Store_hero_location();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Map_ID_Var=ReadCompressedInteger(Stream);
   comand->X_Var=ReadCompressedInteger(Stream);
   comand->Y_Var=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandRecall_to_location(int Command,int Depth,FILE * Stream) {
   Event_comand_Recall_to_location * comand;
   comand = new Event_comand_Recall_to_location();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Map_ID_Var=ReadCompressedInteger(Stream);
   comand->X_Var=ReadCompressedInteger(Stream);
   comand->Y_Var=ReadCompressedInteger(Stream);
   return (comand);
   }

Event_comand * stEvent::EventcommandTeleport_Vehicle(int Command,int Depth,FILE * Stream) {
   Event_comand_Teleport_Vehicle * comand;
   comand = new Event_comand_Teleport_Vehicle();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->type=ReadCompressedInteger(Stream);
   comand->Location=ReadCompressedInteger(Stream);
   comand->Map_ID=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);//2003 6 datos
 if(ChunkInfo.Length==6)
 ReadCompressedInteger(Stream);
   // 2000 5 datos
   return (comand);
   }
Event_comand * stEvent::EventcommandTeleport_Event(int Command,int Depth,FILE * Stream) {
   Event_comand_Teleport_Event * comand;
  comand = new Event_comand_Teleport_Event();
   comand->Comand=Command;
   comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Event_ID=ReadCompressedInteger(Stream);
   comand->By_value=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);//2003 5 datos
   // 2000 4 datos
   if(ChunkInfo.Length==5)
    ReadCompressedInteger(Stream);
      return (comand);
      }
Event_comand * stEvent::EventcommandSwap_Event_Positions(int Command,int Depth,FILE * Stream) {
   Event_comand_Swap_Event_Positions * comand;
  comand = new Event_comand_Swap_Event_Positions();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->First_event=ReadCompressedInteger(Stream);
   comand->Second_event=ReadCompressedInteger(Stream);
      return (comand); }
Event_comand * stEvent::EventcommandGet_Terrain_ID(int Command,int Depth,FILE * Stream) {
   Event_comand_Get_Terrain_ID * comand;
  comand = new Event_comand_Get_Terrain_ID();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->By_value=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);
   comand->Variable_to_store=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandGet_Event_ID(int Command,int Depth,FILE * Stream) {
   Event_comand_Get_Event_ID * comand;
  comand = new Event_comand_Get_Event_ID();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->By_value=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);
   comand->Variable_to_store=ReadCompressedInteger(Stream);
    return (comand); }
Event_comand * stEvent::EventcommandErase_screen(int Command,int Depth,FILE * Stream) {
   Event_comand_Erase_screen * comand;
  comand = new Event_comand_Erase_screen();
   comand->Comand=Command;
   comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Transition=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandShow_screen(int Command,int Depth,FILE * Stream) {
   Event_comand_Show_screen * comand;
   comand = new Event_comand_Show_screen();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Show_screen=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandSet_screen_tone(int Command,int Depth,FILE * Stream) {
   Event_comand_Set_screen_tone * comand;
   comand = new Event_comand_Set_screen_tone();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Red_diffuse=ReadCompressedInteger(Stream);
   comand->Green_diffuse=ReadCompressedInteger(Stream);
   comand->Blue_diffuse=ReadCompressedInteger(Stream);
   comand->Chroma_diffuse=ReadCompressedInteger(Stream);
   comand->Length=ReadCompressedInteger(Stream);
   comand->Wait=ReadCompressedInteger(Stream);
    return (comand); }
Event_comand * stEvent::EventcommandFlash_screen(int Command,int Depth,FILE * Stream) {
   Event_comand_Flash_screen * comand;
  comand = new Event_comand_Flash_screen();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Red_diffuse=ReadCompressedInteger(Stream);
   comand->Green_diffuse=ReadCompressedInteger(Stream);
   comand->Blue_diffuse=ReadCompressedInteger(Stream);
   comand->Strength=ReadCompressedInteger(Stream);
   comand->Length=ReadCompressedInteger(Stream);
   comand->Wait=ReadCompressedInteger(Stream);
   //2000 6 datos 2003 7 datos
   if(ChunkInfo.Length==7)
   ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandShake_screen(int Command,int Depth,FILE * Stream) {
   Event_comand_Shake_screen * comand;
  comand = new Event_comand_Shake_screen();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Power=ReadCompressedInteger(Stream);
   comand->Speed=ReadCompressedInteger(Stream);
   comand->Length=ReadCompressedInteger(Stream);
   comand->Wait=ReadCompressedInteger(Stream);
   //2000 4 datos 2003 5 datos
      if(ChunkInfo.Length==5)
   ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandPan_screen(int Command,int Depth,FILE * Stream) {
   Event_comand_Pan_screen * comand;
  comand = new Event_comand_Pan_screen();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Type=ReadCompressedInteger(Stream);
   comand->Direction=ReadCompressedInteger(Stream);
   comand->Distance=ReadCompressedInteger(Stream);
   comand->Speed=ReadCompressedInteger(Stream);
   comand->Wait=ReadCompressedInteger(Stream);
return (comand);
}
Event_comand * stEvent::EventcommandWeather_Effects(int Command,int Depth,FILE * Stream) {
   Event_comand_Weather_Effects * comand;
  comand = new Event_comand_Weather_Effects();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Type=ReadCompressedInteger(Stream);
   comand->Speed=ReadCompressedInteger(Stream);
   return (comand);
}
Event_comand * stEvent::EventcommandShow_Picture(int Command,int Depth,FILE * Stream) {
   Event_comand_Show_Picture * comand;
  comand = new Event_comand_Show_Picture();
   comand->Comand=Command;
   comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Image_file= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Picture_ID=ReadCompressedInteger(Stream);
   comand->By_Value=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);
   comand->Move_Map=ReadCompressedInteger(Stream);
   comand->Magnification=ReadCompressedInteger(Stream);
   comand->Opacity=ReadCompressedInteger(Stream);
   comand->Use_color_key=ReadCompressedInteger(Stream);
   comand->Red_diffuse=ReadCompressedInteger(Stream);
   comand->Green_diffuse=ReadCompressedInteger(Stream);
   comand->Blue_diffuse=ReadCompressedInteger(Stream);
   comand->Chroma_diffuse=ReadCompressedInteger(Stream);
   comand->Effect=ReadCompressedInteger(Stream);
   comand->Power=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandMove_Picture(int Command,int Depth,FILE * Stream) {
   Event_comand_Move_Picture * comand;
  comand = new Event_comand_Move_Picture();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);

   comand->Picture_ID=ReadCompressedInteger(Stream);
   comand->By_Value=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);
   comand->Opacity=ReadCompressedInteger(Stream);
   comand->Magnification=ReadCompressedInteger(Stream);
   comand->Effect=ReadCompressedInteger(Stream);
   comand->Power=ReadCompressedInteger(Stream);
   comand->Red_diffuse=ReadCompressedInteger(Stream);
   comand->Green_diffuse=ReadCompressedInteger(Stream);
   comand->Blue_diffuse=ReadCompressedInteger(Stream);
   comand->Chroma_diffuse=ReadCompressedInteger(Stream);
   ReadCompressedInteger(Stream);//unknown data
   ReadCompressedInteger(Stream);//unknown data



  if(ChunkInfo.Length>14)
  comand->Length=ReadCompressedInteger(Stream);

  if(ChunkInfo.Length>15)
  comand->Wait=ReadCompressedInteger(Stream);

  if(ChunkInfo.Length>16)
  comand->Wait=ReadCompressedInteger(Stream);

   return (comand); }
Event_comand * stEvent::EventcommandErase_Picture(int Command,int Depth,FILE * Stream) {
   Event_comand_Erase_Picture * comand;
  comand = new Event_comand_Erase_Picture();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Picture_ID=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandShow_Battle_Anim(int Command,int Depth,FILE * Stream) {
   Event_comand_Show_Battle_Anim * comand;
  comand = new Event_comand_Show_Battle_Anim();
    comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Animation_ID=ReadCompressedInteger(Stream);
   comand->Target=ReadCompressedInteger(Stream);
   comand->Wait=ReadCompressedInteger(Stream);
   comand->Full_screen=ReadCompressedInteger(Stream);

   return (comand);
   }

Event_comand * stEvent::EventcommandSet_hero_opacity(int Command,int Depth,FILE * Stream) {
   Event_comand_Set_hero_opacity * comand;
  comand = new Event_comand_Set_hero_opacity();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Opacity=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandFlash_event(int Command,int Depth,FILE * Stream) {
   Event_comand_Flash_event * comand;
  comand = new Event_comand_Flash_event();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Target=ReadCompressedInteger(Stream);
   comand->Red=ReadCompressedInteger(Stream);
   comand->Green=ReadCompressedInteger(Stream);
   comand->Blue=ReadCompressedInteger(Stream);
   comand->Strength=ReadCompressedInteger(Stream);
   comand->Length=ReadCompressedInteger(Stream);
   comand->Wait=ReadCompressedInteger(Stream);
   return(comand);
   }
Event_comand * stEvent::EventcommandMove_event(int Command,int Depth,FILE * Stream) {
   Event_comand_Move_event * comand;
   comand = new Event_comand_Move_event();
   int dat,Length_string;
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Target=ReadCompressedInteger(Stream);
   comand->Frequency=ReadCompressedInteger(Stream);
   comand->Repeat_actions=ReadCompressedInteger(Stream);
   comand->Ignore_impossible=ReadCompressedInteger(Stream);
   // minimo 4 comandos
   ChunkInfo.Length-=4;
   while(ChunkInfo.Length--)
   {
    dat=ReadCompressedInteger(Stream);
        //el 32 activar fase
        //el 33 desactivar fase
        //el 34 es cambiar grafico
        //el 35 es reproduccion de sonido
    switch (dat)
    {
            case 32:
                    Event_comand_Change_switch * comand_key4;
                    comand_key4 = new Event_comand_Change_switch();
                    comand_key4->Comand=dat;
                    comand_key4->Mode=0;
                    comand_key4->toggle_option=0;
                    comand_key4->start_switch=ReadCompressedInteger(Stream);
                    ChunkInfo.Length--;
                    comand->comand_moves.push_back(comand_key4);

                    break;
            case 33:
                    Event_comand_Change_switch * comand_key5;
                    comand_key5 = new Event_comand_Change_switch();
                    comand_key5->Comand=dat;
                    comand_key5->Mode=0;
                    comand_key5->toggle_option=1;
                    comand_key5->start_switch=ReadCompressedInteger(Stream);
                    ChunkInfo.Length--;
                    comand->comand_moves.push_back(comand_key5);
                    break;
            case 34:
                    Event_comand_Change_Hero_Graphic * comand_key1;
                    comand_key1 = new Event_comand_Change_Hero_Graphic();
                    comand_key1->Comand=dat;
                    Length_string=ReadCompressedInteger(Stream);
                    ChunkInfo.Length--;
                    comand_key1->New_graphic =ReadString(Stream,Length_string);
                    ChunkInfo.Length-=Length_string;
                    comand_key1->Sprite_ID=ReadCompressedInteger(Stream);
                    ChunkInfo.Length--;
                    comand->comand_moves.push_back(comand_key1);
            break;
            case 35:
                    Event_comand_Play_SE * comand_key2;
                    comand_key2 = new Event_comand_Play_SE();
                    comand_key2->Comand=dat;
                    Length_string=ReadCompressedInteger(Stream);
                    ChunkInfo.Length--;
                    comand_key2->SE_name =ReadString(Stream,Length_string);
                    ChunkInfo.Length-=Length_string;
                    comand_key2->Volume=ReadCompressedInteger(Stream);
                    comand_key2->Tempo=ReadCompressedInteger(Stream);
                    comand_key2->Balance=ReadCompressedInteger(Stream);
                    ChunkInfo.Length-=3;
                    comand->comand_moves.push_back(comand_key2);
            break;

            default:
                    Event_comand * comand_key3;
                    comand_key3 = new Event_comand();
                    comand_key3->Comand=dat;
                    comand->comand_moves.push_back(comand_key3);
            break;
            }
        }
   return (comand);
   }
Event_comand * stEvent::EventcommandWait(int Command,int Depth,FILE * Stream) {
   Event_comand_Wait * comand;
  comand = new Event_comand_Wait();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Length=ReadCompressedInteger(Stream);
   // 2000 1 dato 2003 2 datos
   if(ChunkInfo.Length==2)
   ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandPlay_BGM(int Command,int Depth,FILE * Stream) {
   Event_comand_Play_BGM * comand;
  comand = new Event_comand_Play_BGM();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->BGM_name= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Fade_in_time=ReadCompressedInteger(Stream);
   comand->Volume=ReadCompressedInteger(Stream);
   comand->Tempo=ReadCompressedInteger(Stream);
   comand->Balance=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandFade_out_BGM(int Command,int Depth,FILE * Stream) {
   Event_comand_Fade_out_BGM * comand;
  comand = new Event_comand_Fade_out_BGM();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Fade_in_time=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandPlay_SE(int Command,int Depth,FILE * Stream) {
   Event_comand_Play_SE * comand;
  comand = new Event_comand_Play_SE();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->SE_name= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Volume=ReadCompressedInteger(Stream);
   comand->Tempo=ReadCompressedInteger(Stream);
   comand->Balance=ReadCompressedInteger(Stream);
   return(comand);
   }
Event_comand * stEvent::EventcommandPlay_movie(int Command,int Depth,FILE * Stream) {
   Event_comand_Play_movie * comand;
  comand = new Event_comand_Play_movie();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Movie_file= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->By_value=ReadCompressedInteger(Stream);
   comand->X=ReadCompressedInteger(Stream);
   comand->Y=ReadCompressedInteger(Stream);
   comand->Width=ReadCompressedInteger(Stream);
   comand->Height=ReadCompressedInteger(Stream);
   return (comand);
   }

Event_comand * stEvent::EventcommandKey_input(int Command,int Depth,FILE * Stream) {
   Event_comand_Key_input * comand;
  comand = new Event_comand_Key_input();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Variable_to_store=ReadCompressedInteger(Stream);
   comand->Wait_for_key=ReadCompressedInteger(Stream);
   comand->Directionals=ReadCompressedInteger(Stream);
   comand->Accept=ReadCompressedInteger(Stream);
   comand->Cancel=ReadCompressedInteger(Stream);
   // unknown data
   if(ChunkInfo.Length>5)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>6)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>7)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>8)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>9)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>10)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>11)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>12)
   ReadCompressedInteger(Stream);
   if(ChunkInfo.Length>13)
   ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandChange_tile(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_tile * comand;
  comand = new Event_comand_Change_tile();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->New_tile=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandChange_background(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_background * comand;
  comand = new Event_comand_Change_background();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Parallax_BG= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->X_pan=ReadCompressedInteger(Stream);
   comand->Y_pan=ReadCompressedInteger(Stream);
   comand->X_auto_pan=ReadCompressedInteger(Stream);
   comand->X_pan_speed=ReadCompressedInteger(Stream);
   comand->Y_auto_pan=ReadCompressedInteger(Stream);
   comand->Y_pan_speed=ReadCompressedInteger(Stream);
   return (comand); }
Event_comand * stEvent::EventcommandChange_encounter_rate(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_encounter_rate * comand;
  comand = new Event_comand_Change_encounter_rate();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Encounter_rate=ReadCompressedInteger(Stream);
   return (comand); }
Event_comand * stEvent::EventcommandChange_single_tile(int Command,int Depth,FILE * Stream) {
    Event_comand_Change_single_tile * comand;
  comand = new Event_comand_Change_single_tile();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Layer=ReadCompressedInteger(Stream);
    comand->Old_tile=ReadCompressedInteger(Stream);
    comand->New_tile=ReadCompressedInteger(Stream);
   return (comand); }
Event_comand * stEvent::EventcommandSet_teleport_location(int Command,int Depth,FILE * Stream) {
    Event_comand_Set_teleport_location * comand;
  comand = new Event_comand_Set_teleport_location();
    comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Add=ReadCompressedInteger(Stream);
    comand->Map_ID=ReadCompressedInteger(Stream);
    comand->X=ReadCompressedInteger(Stream);
    comand->Y=ReadCompressedInteger(Stream);
    comand->Switch=ReadCompressedInteger(Stream);
    comand->Switch_ID=ReadCompressedInteger(Stream);
   return (comand);
   }
Event_comand * stEvent::EventcommandEnable_teleport(int Command,int Depth,FILE * Stream) {
    Event_comand_Enable_teleport * comand;
  comand = new Event_comand_Enable_teleport();
    comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Enable=ReadCompressedInteger(Stream);
    return (comand);
    }

Event_comand * stEvent::EventcommandSet_escape_location(int Command,int Depth,FILE * Stream) {
     Event_comand_Set_escape_location * comand;
  comand = new Event_comand_Set_escape_location();
     comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
     comand->Map_ID=ReadCompressedInteger(Stream);
     comand->X=ReadCompressedInteger(Stream);
     comand->Y=ReadCompressedInteger(Stream);
     comand->Switch=ReadCompressedInteger(Stream);
     comand->Switch_ID=ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandEnable_escape(int Command,int Depth,FILE * Stream) {
    Event_comand_Enable_escape * comand;
  comand = new Event_comand_Enable_escape();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Enable=ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandEnable_saving(int Command,int Depth,FILE * Stream) {
    Event_comand_Enable_saving * comand;
  comand = new Event_comand_Enable_saving();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Enable=ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandEnable_system_menu(int Command,int Depth,FILE * Stream) {
    Event_comand_Enable_system_menu *  comand;
  comand = new Event_comand_Enable_system_menu();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Enable=ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandConditional(int Command,int Depth,FILE * Stream) {
    int size_string;
    Event_comand_Conditional * comand;
    comand = new Event_comand_Conditional();
    comand->Comand=Command;comand->Depth=Depth;
    size_string= ReadCompressedInteger(Stream);
   // printf("men ,%d",size_string);
    if(size_string>0)
    {
    comand->Name_data.clear();
    comand->Name_data.append(ReadString(Stream,size_string));
    }
    else
    ChunkInfo.Length= ReadCompressedInteger(Stream);

    comand->type_of_conditional=ReadCompressedInteger(Stream);
    comand->ID=ReadCompressedInteger(Stream);
    comand->Op_code=ReadCompressedInteger(Stream);
    comand->ID_2=ReadCompressedInteger(Stream);
    comand->Count=ReadCompressedInteger(Stream);
    comand->Exeption=ReadCompressedInteger(Stream);
    return (comand);
    }

Event_comand * stEvent::EventcommandLabel(int Command,int Depth,FILE * Stream) {
    Event_comand_Label * comand;
  comand = new Event_comand_Label();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Label_id=ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandGo_to_label(int Command,int Depth,FILE * Stream) {
    Event_comand_Go_to_label * comand;
  comand = new Event_comand_Go_to_label();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Label_id=ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandCall_event(int Command,int Depth,FILE * Stream) {
    Event_comand_Call_event * comand;
  comand = new Event_comand_Call_event();
    comand->Comand=Command;comand->Depth=Depth;
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Method=ReadCompressedInteger(Stream);
    comand->Event_ID=ReadCompressedInteger(Stream);
    comand->Event_page=ReadCompressedInteger(Stream);
    return (comand);
    }
Event_comand * stEvent::EventcommandComment_Text(int Command,int Depth,FILE * Stream) {
   Event_comand_Comment_Text * comand;
  comand = new Event_comand_Comment_Text();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Text= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
     return (comand);
     }
Event_comand * stEvent::EventcommandAdd_line_to_comment(int Command,int Depth,FILE * Stream) {
   Event_comand_Add_line_to_comment * comand;
  comand = new Event_comand_Add_line_to_comment();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   comand->Text= ReadString(Stream, ChunkInfo.Length);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   return (comand);
   }

Event_comand * stEvent::EventcommandChange_Profetion(int Command,int Depth,FILE * Stream) {
   Event_comand_Change_Profetion * comand;
  comand = new Event_comand_Change_Profetion();
   comand->Comand=Command;comand->Depth=Depth;
   ChunkInfo.Length= ReadCompressedInteger(Stream);
   ChunkInfo.Length= ReadCompressedInteger(Stream);
//7 datos falta 1
    comand->Hero_ID=ReadCompressedInteger(Stream);
    comand->Comands=ReadCompressedInteger(Stream);
    comand->Levels=ReadCompressedInteger(Stream);
    comand->Skills=ReadCompressedInteger(Stream);
    comand->Values=ReadCompressedInteger(Stream);
    comand->Options=ReadCompressedInteger(Stream);
    ReadCompressedInteger(Stream);
     return (comand); }
Event_comand * stEvent::EventcommandChange_Batle_Comands(int Command,int Depth,FILE * Stream) {
    Event_comand_Change_Batle_Comands * comand;
    comand = new Event_comand_Change_Batle_Comands();
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    ChunkInfo.Length= ReadCompressedInteger(Stream);
    comand->Comand=Command;
    comand->Depth=Depth;
    comand->remove=ReadCompressedInteger(Stream);
    comand->Hero_ID=ReadCompressedInteger(Stream);
    comand->Batle_Comand=ReadCompressedInteger(Stream);
    comand->Add=ReadCompressedInteger(Stream);
   return (comand);
}




std:: vector <Event_comand * > stEvent::EventcommandChunk(FILE * Stream)//instrucciones de la paguina
{
         tChunk ChunkInfo; // informacion del pedazo leido
         int data,depth;
         string name;
         std:: vector <Event_comand * > vcEvent_comand;
         data= ReadCompressedInteger(Stream); //de
         depth=ReadCompressedInteger(Stream); //profundidad
         //printf(" data %04X  ",data);
         data =data;

         if(data!=0)
         {       while(data!=0)
                {

      //              printf(" Main data %04X  ",data);
                         switch(data)// tipo de la primera dimencion
                         {
                     case Message:// llamar una funcion con el id que retorne un objeto comando
                           vcEvent_comand.push_back( EventcommandMessageChunk(data,depth,Stream));
                           break;
                     case Add_line_to_message:// 0x819D0E,
                           vcEvent_comand.push_back( EventcommandMessageChunk(data,depth,Stream));
                           break;
                     case Message_options:// 0xCF08,
                          vcEvent_comand.push_back(EventcommandMessageoptionsChunk(data,depth,Stream));
                          break;
                     case Select_message_face:// 0xCF12,
                          vcEvent_comand.push_back(EventcommandSelectfaceChunk(data,depth,Stream));
                          break;
                     case Show_choice:// 0xCF1C,
                          vcEvent_comand.push_back(EventcommandShowchoiceChunk(data,depth,Stream));
                          break;
                     case Nested_block:// 0x0A
                          vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));
                          break;
                     case Show_choice_option:// 0x819D2C,
                          vcEvent_comand.push_back(EventcommandShow_choice_option(data,depth,Stream));
                          break;
                     case End_Choice_block:// 0x819D2D,
                          vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));
                          break;
                     case Number_input:// 0xCF26,
                          vcEvent_comand.push_back(EventcommandNumber_input(data,depth,Stream));
                          break;
                     case Change_switch:// 0xCF62,
                          vcEvent_comand.push_back(EventcommandChange_switch(data,depth,Stream));
                          break;
                     case Change_var:// 0xCF6C,
                          vcEvent_comand.push_back(EventcommandChange_var(data,depth,Stream));
                          break;
                     case Timer_manipulation:// 0xCF76,
                          vcEvent_comand.push_back(EventcommandTimer_manipulation(data,depth,Stream));
                          break;
                     case Change_cash_held:// 0xD046,
                          vcEvent_comand.push_back(EventcommandChange_cash_held(data,depth,Stream));
                          break;
                     case Change_inventory:// 0xD050,
                          vcEvent_comand.push_back(EventcommandChange_inventory(data,depth,Stream));
                          break;
                     case Change_party:// 0xD05A,
                          vcEvent_comand.push_back(EventcommandChange_party(data,depth,Stream));
                          break;
                     case Change_experience:// 0xD12A,
                          vcEvent_comand.push_back(EventcommandChange_experience(data,depth,Stream));
                          break;
                     case Change_level:// 0xD134,
                          vcEvent_comand.push_back(EventcommandChange_level(data,depth,Stream));
                          break;
                     case Change_statistics:// 0xD13E,
                          vcEvent_comand.push_back(EventcommandChange_statistics(data,depth,Stream));
                          break;
                     case Learn_forget_skill:// 0xD148,
                          vcEvent_comand.push_back(EventcommandLearn_forget_skill(data,depth,Stream));
                          break;
                          case Change_equipment:// 0xD152,
                               vcEvent_comand.push_back(EventcommandChange_equipment(data,depth,Stream));
                               break;
                          case Change_HP:// 0xD15C,
                               vcEvent_comand.push_back(EventcommandChange_HP(data,depth,Stream));
                               break;
                          case Change_MP:// 0xD166,
                               vcEvent_comand.push_back(EventcommandChange_MP(data,depth,Stream));

                               break;
                          case Change_Status:// 0xD170,
                               vcEvent_comand.push_back(EventcommandChange_Status(data,depth,Stream));

                               break;
                          case Full_Recovery:// 0xD17A,
                               vcEvent_comand.push_back(EventcommandFull_Recovery(data,depth,Stream));

                               break;
                          case Inflict_Damage:// 0xD204,
                               vcEvent_comand.push_back(EventcommandInflict_Damage(data,depth,Stream));

                               break;
                          case Change_Hero_Name:// 0xD272,
                               vcEvent_comand.push_back(EventcommandChange_Hero_Name(data,depth,Stream));
                               break;
                          case Change_Hero_Class:// 0xD27C,
                               vcEvent_comand.push_back(EventcommandChange_Hero_Class(data,depth,Stream));
                               break;
                          case Change_Hero_Graphic:// 0xD306,
                               vcEvent_comand.push_back(EventcommandChange_Hero_Graphic(data,depth,Stream));

                               break;
                          case Change_Hero_Face:// 0xD310,
                               vcEvent_comand.push_back(EventcommandChange_Hero_Face(data,depth,Stream));

                               break;
                          case Change_Vehicle:// 0xD31A,
                               vcEvent_comand.push_back(EventcommandChange_Vehicle(data,depth,Stream));

                               break;
                          case Change_System_BGM:// 0xD324,
                               vcEvent_comand.push_back(EventcommandChange_System_BGM(data,depth,Stream));

                               break;
                          case Change_System_SE:// 0xD32E,
                               vcEvent_comand.push_back(EventcommandChange_System_SE(data,depth,Stream));

                               break;
                          case Change_System_GFX:// 0xD338,
                               vcEvent_comand.push_back(EventcommandChange_System_GFX(data,depth,Stream));

                               break;
                          case Change_Transition:// 0xD342,
                               vcEvent_comand.push_back(EventcommandChange_Transition(data,depth,Stream));

                               break;
                          case Start_Combat:// 0xD356,
                               vcEvent_comand.push_back(EventcommandStart_Combat(data,depth,Stream));
                               break;

                          case Call_Shop:// 0xD360,
                               vcEvent_comand.push_back(EventcommandCall_Shop(data,depth,Stream));
 //instruccion estandar

                               break;
                          case Start_success_block:// 0x81A170,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Start_failure_block:// 0x81A171,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case End_shop_block:// 0x81A172,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;

                          case Call_Inn:// 0xD36A,
                               vcEvent_comand.push_back(EventcommandCall_Inn(data,depth,Stream));

                               break;
                          case Start_success_block2:// 0x81A17A,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Start_failure_block2:// 0x81A17B,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case End_block:// 0x81A17C,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Enter_hero_name:// 0xD374,
                               vcEvent_comand.push_back(EventcommandEnter_hero_name(data,depth,Stream));

                               break;
                          case Teleport_Party:// 0xD34A	,
                               vcEvent_comand.push_back(EventcommandTeleport_Party(data,depth,Stream));
                              break;
                          case Store_hero_location:// 0xD444,
                               vcEvent_comand.push_back(EventcommandStore_hero_location(data,depth,Stream));

                               break;
                          case Recall_to_location:// 0xD44E,
                               vcEvent_comand.push_back(EventcommandRecall_to_location(data,depth,Stream));

                               break;
                          case Ride_Dismount:// 0xD458	,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Teleport_Vehicle:// 0xD462,
                               vcEvent_comand.push_back(EventcommandTeleport_Vehicle(data,depth,Stream));

                               break;
                          case Teleport_Event:// 0xD46C,
                               vcEvent_comand.push_back(EventcommandTeleport_Event(data,depth,Stream));

                               break;
                          case Swap_Event_Positions:// 0xD476,
                               vcEvent_comand.push_back(EventcommandSwap_Event_Positions(data,depth,Stream));

                               break;
                          case Get_Terrain_ID:// 0xD51E,
                               vcEvent_comand.push_back(EventcommandGet_Terrain_ID(data,depth,Stream));

                               break;
                          case Get_Event_ID:// 0xD528,
                               vcEvent_comand.push_back(EventcommandGet_Event_ID(data,depth,Stream));

                               break;
                          case Erase_screen:// 0xD602,
                               vcEvent_comand.push_back(EventcommandErase_screen(data,depth,Stream));

                               break;
                          case Show_screen:// 0xD60C,
                               vcEvent_comand.push_back(EventcommandShow_screen(data,depth,Stream));

                               break;
                          case Set_screen_tone:// 0xD616,
                               vcEvent_comand.push_back(EventcommandSet_screen_tone(data,depth,Stream));

                               break;
                          case Flash_screen:// 0xD620,
                               vcEvent_comand.push_back(EventcommandFlash_screen(data,depth,Stream));

                               break;
                          case Shake_screen:// 0xD62A,
                               vcEvent_comand.push_back(EventcommandShake_screen(data,depth,Stream));

                               break;
                          case Pan_screen:// 0xD634,
                               vcEvent_comand.push_back(EventcommandPan_screen(data,depth,Stream));

                               break;
                          case Weather_Effects:// 0xD63E,
                               vcEvent_comand.push_back(EventcommandWeather_Effects(data,depth,Stream));

                               break;
                          case Show_Picture:// 0xD666,
                               vcEvent_comand.push_back(EventcommandShow_Picture(data,depth,Stream));

                               break;
                         case Move_Picture:// 0xD670,
                               vcEvent_comand.push_back(EventcommandMove_Picture(data,depth,Stream));
                               break;
                          case Erase_Picture:// 0xD67A,
                               vcEvent_comand.push_back(EventcommandErase_Picture(data,depth,Stream));

                               break;
                          case Show_Battle_Anim :// 0xD74A,
                               vcEvent_comand.push_back(EventcommandShow_Battle_Anim(data,depth,Stream));

                               break;
                          case Set_hero_opacity:// 0xD82E,
                               vcEvent_comand.push_back(EventcommandSet_hero_opacity(data,depth,Stream));

                               break;
                          case Flash_event:// 0xD838,
                               vcEvent_comand.push_back(EventcommandFlash_event(data,depth,Stream));

                               break;
                          case Move_event:// 0xD842,
                               vcEvent_comand.push_back(EventcommandMove_event(data,depth,Stream));

                               break;
                          case Wait_until_moved:// 0xD84C,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Stop_all_movement:// 0xD856,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Wait:// 0xD912,
                               vcEvent_comand.push_back(EventcommandWait(data,depth,Stream));

                               break;
                          case Play_BGM:// 0xD976,
                               vcEvent_comand.push_back(EventcommandPlay_BGM(data,depth,Stream));

                               break;
                          case Fade_out_BGM:// 0xDA00,
                               vcEvent_comand.push_back(EventcommandFade_out_BGM(data,depth,Stream));

                               break;
                          case Memorize_BGM:// 0xDA0A,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Play_memorized:// 0xDA14,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Play_sound_effect:// 0xDA1E,
                               vcEvent_comand.push_back(EventcommandPlay_SE(data,depth,Stream));

                               break;
                          case Play_movie:// 0xDA28,
                               vcEvent_comand.push_back(EventcommandPlay_movie(data,depth,Stream));

                               break;
                          case Key_input:// 0xDA5A,
                               vcEvent_comand.push_back(EventcommandKey_input(data,depth,Stream));

                               break;
                          case Change_tile_set:// 0xDB3E	,
                               vcEvent_comand.push_back(EventcommandChange_tile(data,depth,Stream));

                               break;
                          case Change_background:// 0xDB48,
                               vcEvent_comand.push_back(EventcommandChange_background(data,depth,Stream));

                               break;
                          case Change_encounter_rate:// 0xDB5C,
                               vcEvent_comand.push_back(EventcommandChange_encounter_rate(data,depth,Stream));

                               break;
                          case Change_single_tile:// 0xDB66,
                               vcEvent_comand.push_back(EventcommandChange_single_tile(data,depth,Stream));

                               break;
                          case Set_teleport_location:// 0xDC22,
                               vcEvent_comand.push_back(EventcommandSet_teleport_location(data,depth,Stream));

                               break;
                          case Enable_teleport:// 0xDC2C,
                               vcEvent_comand.push_back(EventcommandEnable_teleport(data,depth,Stream));

                               break;
                          case Set_escape_location:// 0xDC36,
                               vcEvent_comand.push_back(EventcommandSet_escape_location(data,depth,Stream));

                               break;
                          case Enable_escape:// 0xDC40,
                               vcEvent_comand.push_back(EventcommandEnable_escape(data,depth,Stream));

                               break;
                          case Call_save_menu:// 0xDD06,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Enable_saving:// 0xDD1A,
                               vcEvent_comand.push_back(EventcommandEnable_saving(data,depth,Stream));

                               break;
                          case Call_system_menu:// 0xDD2E,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Enable_system_menu:// 0xDD38,
                               vcEvent_comand.push_back(EventcommandEnable_system_menu(data,depth,Stream));

                               break;
                          case Conditional:// 0xDD6A,
                               vcEvent_comand.push_back(EventcommandConditional(data,depth,Stream));

                               break;
                          case Else_case:// 0x81AB7A,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case End_conditional:// 0x81AB7B,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Label:// 0xDE4E,
                               vcEvent_comand.push_back(EventcommandLabel(data,depth,Stream));

                               break;
                          case Go_to_label:// 0xDE58,
                               vcEvent_comand.push_back(EventcommandGo_to_label(data,depth,Stream));

                               break;
                          case Start_loop:// 0xDF32,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case End_loop:// 0x81AD42,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Break:// 0xDF3C,
                                vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Stop_all_events:// 0xE016,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Delete_event:// 0xE020,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Call_event:// 0xE02A,
                               vcEvent_comand.push_back(EventcommandCall_event(data,depth,Stream));

                               break;
                          case Comment:// 0xE07A,
                               vcEvent_comand.push_back(EventcommandComment_Text(data,depth,Stream));

                               break;
                          case Add_line_to_comment:// 0x81AF0A,
                               vcEvent_comand.push_back(EventcommandAdd_line_to_comment(data,depth,Stream));

                               break;
                          case Game_over:// 0xE104,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Return_to_title_screen:// 0xE15E,
                               vcEvent_comand.push_back(EventcommandSimpleEvent_comand(data,depth,Stream));

                               break;
                          case Change_Profetion:// 0xE104,
                               vcEvent_comand.push_back(EventcommandChange_Profetion(data,depth,Stream));

                               break;
                          case Change_Batle_Comands:// 0xE15E,
                               vcEvent_comand.push_back(EventcommandChange_Batle_Comands(data,depth,Stream));

                               break;
                          case End_of_event:
                               break;
                          default:
                               //instruccion estandar
                           ChunkInfo.Length= ReadCompressedInteger(Stream); //primera longitud
                           name = ReadString(Stream, ChunkInfo.Length);
//                             printf(name.c_str());
                            ChunkInfo.Length= ReadCompressedInteger(Stream); //segunda longitud
                            while(ChunkInfo.Length--)//seguro longitud
                            {data= ReadCompressedInteger(Stream);
                              printf("X  %d ",data);
                             printf(" %c ",data);

                             }
                           break;
                          }
                          data     = ReadCompressedInteger(Stream);  // lectura de tipo del pedazo
                        if(data!=0)
                         depth= ReadCompressedInteger(Stream); //profundidad
              }
               //printf("\n");
              // 3 bytes, tamaño estandar de instrucion
             ReadCompressedInteger(Stream); // final de cadena
             ReadCompressedInteger(Stream); // final de cadena
             ReadCompressedInteger(Stream); // final de cadena
        }
        else{
              ReadCompressedInteger(Stream); // final de cadena
         ReadCompressedInteger(Stream); // final de cadena
	}
return(vcEvent_comand);
}
