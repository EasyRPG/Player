#include "Event_management.h"


void E_management::init(Audio * audio,unsigned char * TheScene,Player_Team * TheTeam,std:: vector <stEventMap> * TheEvents, std:: vector <Chara> * TheCharas_nps,CActor * TheActor, map_data * Thedata,Chipset * the_chip)
{
     std::string system_string;
    chip= the_chip;
    NScene=TheScene;
	myaudio=audio;
    myteam =TheTeam;
    Events=TheEvents;
    Charas_nps=TheCharas_nps;
    Actor= TheActor;
    Thedata= data;
    system_string.append("System/");
    system_string.append(TheTeam->data2.System_dat.System_graphic);
    system_string.append(".png");

    message_box.init(320, 80, 0, 160, system_string.c_str());
    message_box.visible = false;


}

void E_management::update(SDL_Surface *Screen)
{
 message_box.draw(Screen);
}


int E_management::exec_comand(Event_comand * comand,int event_id, int comand_id)
{

    switch (comand->Comand)
    {
    case Message:
        Event_comand_Message * comand_Message;
        comand_Message= ( Event_comand_Message *)comand;
        message_box.add_text(comand_Message->Text, 9, 9);
        message_box.visible = true;

        break;
    case Add_line_to_message:
        printf("Addline texto");
        Event_comand_Message * Add_line_to;
        Add_line_to= ( Event_comand_Message *)comand;
        printf("\n texto %s",Add_line_to->Text.c_str());
        break;
    case Message_options:// 0xCF08,
        Event_comand_Message_options * comand_Message_options;
        comand_Message_options = (Event_comand_Message_options *)comand;
        break;
    case Select_message_face:// 0xCF12,
        Event_comand_Select_face * comand_Select_face;
        comand_Select_face = (Event_comand_Select_face *)comand;
        break;
    case Show_choice:// 0xCF1C,
        Event_comand_Show_choice * comand_Show_choice;
        comand_Show_choice = (Event_comand_Show_choice *)comand;
        break;
    case Nested_block:// 0x0A
        break;
    case Show_choice_option:// 0x819D2C,
        Event_comand_Show_choice_option * comand_Show_choice_option;
        comand_Show_choice_option = (Event_comand_Show_choice_option *)comand;
        break;
    case End_Choice_block:// 0x819D2D,
        break;
    case Number_input:// 0xCF26,
        Event_comand_Number_input * comand_Number_input;
        comand_Number_input = (Event_comand_Number_input *)comand;

        break;
    case Change_switch:// 0xCF62,
        Event_comand_Change_switch * comand_Change_switch;
        comand_Change_switch = (Event_comand_Change_switch *)comand;
        break;
    case Change_var:// 0xCF6C,
        Event_comand_Change_var * comand_Change_var;
        comand_Change_var = (Event_comand_Change_var *)comand;
        break;
    case Timer_manipulation:// 0xCF76,
        Event_comand_Timer_manipulation * comand_Timer_manipulation;
        comand_Timer_manipulation = (Event_comand_Timer_manipulation *)comand;
        break;
    case Change_cash_held:// 0xD046,
        Event_comand_Change_cash_held * comand_Change_cash_held;
        comand_Change_cash_held = (Event_comand_Change_cash_held *)comand;
        break;
    case Change_inventory:// 0xD050,
        Event_comand_Change_inventory * comand_Change_inventory;
        comand_Change_inventory = (Event_comand_Change_inventory *)comand;
        break;
    case Change_party:// 0xD05A,
        Event_comand_Change_party * comand_Change_party;
        comand_Change_party = (Event_comand_Change_party *)comand;
        break;
    case Change_experience:// 0xD12A,
        Event_comand_Change_experience * comand_Change_experience;
        comand_Change_experience = (Event_comand_Change_experience *)comand;
        break;
    case Change_level:// 0xD134,
        Event_comand_Change_level * comand_Change_level;
        comand_Change_level = (Event_comand_Change_level *)comand;
        break;
    case Change_statistics:// 0xD13E,
        Event_comand_Change_statistics * comand_Change_statistics;
        comand_Change_statistics = (Event_comand_Change_statistics *)comand;
        break;
    case Learn_forget_skill:// 0xD148,
        Event_comand_Learn_forget_skill * comand_Learn_forget_skill;
        comand_Learn_forget_skill = (Event_comand_Learn_forget_skill *)comand;
        break;
    case Change_equipment:// 0xD152,
        Event_comand_Change_equipment * comand_Change_equipment;
        comand_Change_equipment = (Event_comand_Change_equipment *)comand;
        break;
    case Change_HP:// 0xD15C,
        Event_comand_Change_HP * comand_Change_HP;
        comand_Change_HP = (Event_comand_Change_HP *)comand;
        break;
    case Change_MP:// 0xD166,
        Event_comand_Change_MP * comand_Change_MP;
        comand_Change_MP = (Event_comand_Change_MP *)comand;
        break;
    case Change_Status:// 0xD170,
        Event_comand_Change_Status * comand_Change_Status;
        comand_Change_Status = (Event_comand_Change_Status *)comand;
        break;
    case Full_Recovery:// 0xD17A,
        Event_comand_Full_Recovery * comand_Full_Recovery;
        comand_Full_Recovery = (Event_comand_Full_Recovery *)comand;
        break;
    case Inflict_Damage:// 0xD204,
        Event_comand_Inflict_Damage * comand_Inflict_Damage;
        comand_Inflict_Damage = (Event_comand_Inflict_Damage *)comand;
        break;
    case Change_Hero_Name:// 0xD272,
        Event_comand_Change_Hero_Name * comand_Change_Hero_Name;
        comand_Change_Hero_Name = (Event_comand_Change_Hero_Name * )comand;
        break;
    case Change_Hero_Class:// 0xD27C,
        Event_comand_Change_Hero_Class * comand_Change_Hero_Class;
        comand_Change_Hero_Class = (Event_comand_Change_Hero_Class *)comand;
        break;
    case Change_Hero_Graphic:// 0xD306,
        Event_comand_Change_Hero_Graphic * comand_Change_Hero_Graphic;
        comand_Change_Hero_Graphic = (Event_comand_Change_Hero_Graphic *)comand;
        break;
    case Change_Hero_Face:// 0xD310,
        Event_comand_Change_Hero_Face * comand_Change_Hero_Face;
        comand_Change_Hero_Face = (Event_comand_Change_Hero_Face *)comand;
        break;
    case Change_Vehicle:// 0xD31A,
        Event_comand_Change_Vehicle * comand_Change_Vehicle;
        comand_Change_Vehicle = (Event_comand_Change_Vehicle *)comand;
        break;
    case Change_System_BGM:// 0xD324,
        Event_comand_Change_System_BGM * comand_Change_System_BGM;
        comand_Change_System_BGM = (Event_comand_Change_System_BGM *)comand;
        break;
    case Change_System_SE:// 0xD32E,
        Event_comand_Change_System_SE * comand_Change_System_SE;
        comand_Change_System_SE = (Event_comand_Change_System_SE *)comand;
        break;
    case Change_System_GFX:// 0xD338,
        Event_comand_Change_System_GFX * comand_Change_System_GFX;
        comand_Change_System_GFX= (Event_comand_Change_System_GFX *)comand;
        break;
    case Change_Transition:// 0xD342,
        Event_comand_Change_Transition * comand_Change_Transition;
        comand_Change_Transition = (Event_comand_Change_Transition *)comand;
        break;
    case Start_Combat:// 0xD356,
        Event_comand_Start_Combat * comand_Start_Combat;
        comand_Start_Combat = (Event_comand_Start_Combat*)comand;
        break;

    case Call_Shop:// 0xD360,
        Event_comand_Call_Shop * comand_Call_Shop;
        comand_Call_Shop = (Event_comand_Call_Shop*)comand;
        break;
    case Start_success_block:// 0x81A170,
        break;
    case Start_failure_block:// 0x81A171,

        break;
    case End_shop_block:// 0x81A172,

        break;

    case Call_Inn:// 0xD36A,
        Event_comand_Call_Inn * comand_Call_Inn;
        comand_Call_Inn = (Event_comand_Call_Inn*)comand;
        break;
    case Start_success_block2:// 0x81A17A,

        break;
    case Start_failure_block2:// 0x81A17B,

        break;
    case End_block:// 0x81A17C,

        break;
    case Enter_hero_name:// 0xD374,
        Event_comand_Enter_hero_name * comand_Enter_hero_name;
        comand_Enter_hero_name =(Event_comand_Enter_hero_name *)comand;
        break;
    case Teleport_Party:
        Event_comand_Teleport_Party * command_Teleport_Party;
        command_Teleport_Party= ( Event_comand_Teleport_Party *)comand;
        myteam->actual_map=command_Teleport_Party->Map_ID;
        myteam->actual_x_map=command_Teleport_Party->X;
        myteam->actual_y_map=command_Teleport_Party->Y;
        Actor->setposXY(myteam->actual_x_map,myteam->actual_y_map);

        break;
    case Store_hero_location:// 0xD444,
        Event_comand_Store_hero_location * comand_Store_hero_location;
        comand_Store_hero_location=(Event_comand_Store_hero_location *)comand;
        break;
    case Recall_to_location:// 0xD44E,
        Event_comand_Recall_to_location * comand_Recall_to_location;
        comand_Recall_to_location=(Event_comand_Recall_to_location *)comand;
        break;
    case Ride_Dismount:// 0xD458	,
        break;
    case Teleport_Vehicle:// 0xD462,
        Event_comand_Teleport_Vehicle * comand_Teleport_Vehicle;
        comand_Teleport_Vehicle=(Event_comand_Teleport_Vehicle *)comand;

        break;
    case Teleport_Event:// 0xD46C,
        Event_comand_Teleport_Event * comand_Teleport_Event;
        comand_Teleport_Event=(Event_comand_Teleport_Event *)comand;

        break;
    case Swap_Event_Positions:// 0xD476,
        Event_comand_Swap_Event_Positions * comand_Swap_Event_Positions;
        comand_Swap_Event_Positions=(Event_comand_Swap_Event_Positions *)comand;

        break;
    case Get_Terrain_ID:// 0xD51E,
        Event_comand_Get_Terrain_ID * comand_Get_Terrain_ID;
        comand_Get_Terrain_ID=(Event_comand_Get_Terrain_ID *)comand;

        break;
    case Get_Event_ID:// 0xD528,
        Event_comand_Get_Event_ID * comand_Get_Event_ID;
        comand_Get_Event_ID=(Event_comand_Get_Event_ID *)comand;

        break;
    case Erase_screen:// 0xD602,
        Event_comand_Erase_screen * comand_Erase_screen;
        comand_Erase_screen=(Event_comand_Erase_screen *)comand;

        break;
    case Show_screen:// 0xD60C,
        Event_comand_Show_screen * comand_Show_screen;
        comand_Show_screen=(Event_comand_Show_screen *)comand;

        break;
    case Set_screen_tone:// 0xD616,
        Event_comand_Set_screen_tone * comand_Set_screen_tone;
        comand_Set_screen_tone=(Event_comand_Set_screen_tone *)comand;

        break;
    case Flash_screen:// 0xD620,
        Event_comand_Flash_screen * comand_Flash_screen;
        comand_Flash_screen=(Event_comand_Flash_screen *)comand;

        break;
    case Shake_screen:// 0xD62A,
        Event_comand_Shake_screen * comand_Shake_screen;
        comand_Shake_screen=(Event_comand_Shake_screen *)comand;

        break;
    case Pan_screen:// 0xD634,
        Event_comand_Pan_screen * comand_Pan_screen;
        comand_Pan_screen=(Event_comand_Pan_screen *)comand;

        break;
    case Weather_Effects:// 0xD63E,
        Event_comand_Weather_Effects * comand_Weather_Effects;
        comand_Weather_Effects=(Event_comand_Weather_Effects *)comand;
        break;
    case Show_Picture:// 0xD666,
        Event_comand_Show_Picture * comand_Show_Picture;
        comand_Show_Picture=(Event_comand_Show_Picture *)comand;

        break;
    case Move_Picture:// 0xD670,
        Event_comand_Move_Picture * comand_Move_Picture;
        comand_Move_Picture=(Event_comand_Move_Picture *)comand;

        break;
    case Erase_Picture:// 0xD67A,
        Event_comand_Erase_Picture * comand_Erase_Picture;
        comand_Erase_Picture=(Event_comand_Erase_Picture *)comand;
        break;
    case Show_Battle_Anim :// 0xD74A,
        Event_comand_Show_Battle_Anim * comand_Show_Battle_Anim;
        comand_Show_Battle_Anim=(Event_comand_Show_Battle_Anim *)comand;
        break;
    case Set_hero_opacity:// 0xD82E,
        Event_comand_Set_hero_opacity * comand_Set_hero_opacity;
        comand_Set_hero_opacity=(Event_comand_Set_hero_opacity *)comand;

        break;
    case Flash_event:// 0xD838,
        Event_comand_Flash_event * comand_Flash_event;
        comand_Flash_event=(Event_comand_Flash_event *)comand;

        break;
    case Move_event:// 0xD842,
        Event_comand_Move_event * comand_Move_event;
        comand_Move_event=(Event_comand_Move_event *)comand;

        break;
    case Wait_until_moved:// 0xD84C,
        break;
    case Stop_all_movement:// 0xD856,
        break;
    case Wait:// 0xD912,
        Event_comand_Wait * comand_Wait;
        comand_Wait=(Event_comand_Wait *)comand;

        break;
    case Play_BGM:// 0xD976,
        Event_comand_Play_BGM * comand_Play_BGM;
        comand_Play_BGM=(Event_comand_Play_BGM *)comand;

        break;
    case Fade_out_BGM:// 0xDA00,
        Event_comand_Fade_out_BGM * comand_Fade_out_BGM;
        comand_Fade_out_BGM=(Event_comand_Fade_out_BGM *)comand;

        break;
    case Memorize_BGM:// 0xDA0A,
        break;
    case Play_memorized:// 0xDA14,

        break;
    case Play_sound_effect:// 0xDA1E,

        Event_comand_Play_SE * comand_Play_SE;
        comand_Play_SE=(Event_comand_Play_SE *)comand;
        break;
    case Play_movie:// 0xDA28,

        Event_comand_Play_movie * comand_Play_movie;
        comand_Play_movie=(Event_comand_Play_movie *)comand;
        break;
    case Key_input:// 0xDA5A,
        Event_comand_Key_input * comand_Key_input;
        comand_Key_input=(Event_comand_Key_input *)comand;

        break;
    case Change_tile_set:// 0xDB3E	,
        Event_comand_Change_tile * comand_Change_tile;
        comand_Change_tile=(Event_comand_Change_tile *)comand;

        break;
    case Change_background:// 0xDB48,
        Event_comand_Change_background * comand_Change_background;
        comand_Change_background=(Event_comand_Change_background *)comand;

        break;
    case Change_encounter_rate:
        Event_comand_Change_encounter_rate * command_Change_encounter_rate;
        command_Change_encounter_rate= ( Event_comand_Change_encounter_rate *)comand;
        myteam->Encounter_rate=command_Change_encounter_rate->Encounter_rate;
        break;
    case Change_single_tile:// 0xDB66,
        Event_comand_Change_single_tile * comand_Change_single_tile;
        comand_Change_single_tile=(Event_comand_Change_single_tile*)comand;
        break;
    case Set_teleport_location:// 0xDC22,
        Event_comand_Set_teleport_location * comand_Set_teleport_location;
        comand_Set_teleport_location = (Event_comand_Set_teleport_location*)comand;
        break;
    case Enable_teleport:
        Event_comand_Enable_teleport * comand_Enable_teleport;
        comand_Enable_teleport= (Event_comand_Enable_teleport *)comand;
        myteam->able_to_teleport=comand_Enable_teleport->Enable;
        printf("Enable_teleport %d",comand_Enable_teleport->Enable);
        break;
    case Set_escape_location:// 0xDC36,
        Event_comand_Set_escape_location * comand_Set_escape_location;
        comand_Set_escape_location=(Event_comand_Set_escape_location *)comand;
        break;
    case Enable_escape:
        Event_comand_Enable_escape * comand_Enable_escape;
        comand_Enable_escape= (Event_comand_Enable_escape *) comand;
        myteam->able_to_escape=comand_Enable_escape->Enable;
        printf("Enable_saving %d",comand_Enable_escape->Enable);
        break;
    case Call_save_menu:
        myteam->actual_x_map=Actor->GridX;
        myteam->actual_y_map=Actor->GridY;
        myteam->actual_dir=Actor->get_dir();
        *NScene = 9;
        break;
    case Enable_saving:
        Event_comand_Enable_saving * comand_Enable_saving;
        comand_Enable_saving= (Event_comand_Enable_saving *) comand;
        myteam->able_to_save=comand_Enable_saving->Enable;
        printf("Enable_saving %d",comand_Enable_saving->Enable);
        break;
    case Call_system_menu:
        myteam->actual_x_map=Actor->GridX;
        myteam->actual_y_map=Actor->GridY;
        myteam->actual_dir=Actor->get_dir();
        *NScene = 4;
        break;
    case Enable_system_menu:
        Event_comand_Enable_system_menu * comand_Enable_system_menu;
        comand_Enable_system_menu= (Event_comand_Enable_system_menu *) comand;
        myteam->able_to_menu=comand_Enable_system_menu->Enable;
        printf("Enable_system_menu %d",comand_Enable_system_menu->Enable);
        break;
    case Conditional:// 0xDD6A,
        Event_comand_Conditional * comand_Conditional;
        comand_Conditional= (Event_comand_Conditional *)comand;
        break;
    case Else_case:// 0x81AB7A,

        break;
    case End_conditional:// 0x81AB7B,

        break;
    case Label:// 0xDE4E,
        Event_comand_Label * comand_Label;
        comand_Label = (Event_comand_Label *)comand;
        break;
    case Go_to_label:// 0xDE58,
        Event_comand_Go_to_label * comand_Go_to_label;
        comand_Go_to_label = (Event_comand_Go_to_label *)comand;
        break;
    case Start_loop:// 0xDF32,

        break;
    case End_loop:// 0x81AD42,

        break;
    case Break:// 0xDF3C,

        break;
    case Stop_all_events:// 0xE016,

        break;
    case Delete_event:
        Charas_nps->at(event_id).GridX= data->MapHeight;
        Charas_nps->at(event_id).GridY= data->MapWidth;
        Charas_nps->at(event_id).layer= 3;
        break;
    case Call_event:// 0xE02A,
        Event_comand_Call_event * comand_Call_event;
        comand_Call_event= (Event_comand_Call_event *)comand;
        break;
    case Comment:// 0xE07A,

        break;
    case Add_line_to_comment:// 0x81AF0A,

        break;
    case Game_over:
        *NScene = 3;
        break;
    case Return_to_title_screen:
        *NScene = 0;
        break;



    }
return(comand_id);
}
