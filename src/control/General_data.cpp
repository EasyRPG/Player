#include "Event_management.h"

General_data::General_data()
{
    running = true;
    TheScene = 0;
    
    ldbdata = new LDB_data();
    if (ldbdata == NULL)
    {
        std::cerr << "No memory left allocating LDB data." << std::endl;
        exit(-1);
    }

    before_telepor.Event_Active=false;
    before_telepor.id_exe_actual=0;
    before_telepor.id_actual_active=false;
    before_telepor.Active_page= -2;//common event

}

General_data::~General_data()
{
    //std::cout << "Destructor general data" << std::endl;
    delete ldbdata;
}

void General_data::copy_to_before_telepor(std:: vector <Event_comand *> comands)
{
unsigned int i;
unsigned int size;

comand_before_telepor.clear();
Event_comand * comand;


for(i=0; i<comands.size();i++)
{
    switch (comands[i]->Comand)
    {
    case Message:
        size=sizeof(Event_comand_Message);
        break;
    case Message_options:// 0xCF08,
        size=sizeof(Event_comand_Message_options );// comand_Message_options;
        break;
    case Select_message_face:// 0xCF12,
        size=sizeof(Event_comand_Select_face );// comand_Select_face;
        break;
    case Show_choice:// 0xCF1C,
        size=sizeof(Event_comand_Show_choice );// comand_Show_choice;
        break;
    case Show_choice_option:// 0x819D2C,
        size=sizeof(Event_comand_Show_choice_option );// comand_Show_choice_option;
        break;
    case Number_input:// 0xCF26,
        size=sizeof(Event_comand_Number_input );// comand_Number_input;
        break;
    case Change_switch:// 0xCF62,
        size=sizeof(Event_comand_Change_switch );// comand_Change_switch;
        break;
    case Change_var:// 0xCF6C,
        size=sizeof(Event_comand_Change_var );// comand_Change_var;
        break;
    case Timer_manipulation:// 0xCF76,
        size=sizeof(Event_comand_Timer_manipulation );// comand_Timer_manipulation;
        break;
    case Change_cash_held:// 0xD046,
        size=sizeof(Event_comand_Change_cash_held );// comand_Change_cash_held;
        break;
    case Change_inventory:// 0xD050,
        size=sizeof(Event_comand_Change_inventory );// comand_Change_inventory;
        break;
    case Change_party:// 0xD05A,
        size=sizeof(Event_comand_Change_party );// comand_Change_party;
        break;
    case Change_experience:// 0xD12A,
        size=sizeof(Event_comand_Change_experience );// comand_Change_experience;
        break;
    case Change_level:// 0xD134,
        size=sizeof(Event_comand_Change_level );// comand_Change_level;
        break;
    case Change_statistics:// 0xD13E,
        size=sizeof(Event_comand_Change_statistics );// comand_Change_statistics;
        break;
    case Learn_forget_skill:// 0xD148,
        size=sizeof(Event_comand_Learn_forget_skill );// comand_Learn_forget_skill;
        break;
    case Change_equipment:// 0xD152,
        size=sizeof(Event_comand_Change_equipment );// comand_Change_equipment;
        break;
    case Change_HP:// 0xD15C,
        size=sizeof(Event_comand_Change_HP );// comand_Change_HP;
        break;
    case Change_MP:// 0xD166,
        size=sizeof(Event_comand_Change_MP );// comand_Change_MP;
        break;
    case Change_Status:// 0xD170,
        size=sizeof(Event_comand_Change_Status );// comand_Change_Status;
        break;
    case Full_Recovery:// 0xD17A,
        size=sizeof(Event_comand_Full_Recovery );// comand_Full_Recovery;
        break;
    case Inflict_Damage:// 0xD204,
        size=sizeof(Event_comand_Inflict_Damage );// comand_Inflict_Damage;
        break;
    case Change_Hero_Name:// 0xD272,
        size=sizeof(Event_comand_Change_Hero_Name );// comand_Change_Hero_Name;
        break;
    case Change_Hero_Class:// 0xD27C,
        size=sizeof(Event_comand_Change_Hero_Class );// comand_Change_Hero_Class;
        break;
    case Change_Hero_Graphic:// 0xD306,
        size=sizeof(Event_comand_Change_Hero_Graphic );// comand_Change_Hero_Graphic;
        break;
    case Change_Hero_Face:// 0xD310,
        size=sizeof(Event_comand_Change_Hero_Face );// comand_Change_Hero_Face;
        break;
    case Change_Vehicle:// 0xD31A,
        size=sizeof(Event_comand_Change_Vehicle );// comand_Change_Vehicle;
        break;
    case Change_System_BGM:// 0xD324,
        size=sizeof(Event_comand_Change_System_BGM );// comand_Change_System_BGM;
        break;
    case Change_System_SE:// 0xD32E,
        size=sizeof(Event_comand_Change_System_SE );// comand_Change_System_SE;
        break;
    case Change_System_GFX:// 0xD338,
        size=sizeof(Event_comand_Change_System_GFX );// comand_Change_System_GFX;
        break;
    case Change_Transition:// 0xD342,
        size=sizeof(Event_comand_Change_Transition );// comand_Change_Transition;
        break;
    case Start_Combat:// 0xD356,
        size=sizeof(Event_comand_Start_Combat );// comand_Start_Combat;
        break;
    case Call_Shop:// 0xD360,
        size=sizeof(Event_comand_Call_Shop );// comand_Call_Shop;
        break;
    case Call_Inn:// 0xD36A,
        size=sizeof(Event_comand_Call_Inn );// comand_Call_Inn;
        break;
    case Enter_hero_name:// 0xD374,
        size=sizeof(Event_comand_Enter_hero_name );// comand_Enter_hero_name;
        break;
    case Store_hero_location:// 0xD444,
        size=sizeof(Event_comand_Store_hero_location );// comand_Store_hero_location;
        break;
    case Recall_to_location:// 0xD44E,
        size=sizeof(Event_comand_Recall_to_location );// comand_Recall_to_location;
        break;
    case Teleport_Vehicle:// 0xD462,
        size=sizeof(Event_comand_Teleport_Vehicle );// comand_Teleport_Vehicle;
        break;
    case Teleport_Event:// 0xD46C,
        size=sizeof(Event_comand_Teleport_Event );// comand_Teleport_Event;
        break;
    case Swap_Event_Positions:// 0xD476,
        size=sizeof(Event_comand_Swap_Event_Positions );// comand_Swap_Event_Positions;
        break;
    case Get_Terrain_ID:// 0xD51E,
        size=sizeof(Event_comand_Get_Terrain_ID );// comand_Get_Terrain_ID;
        break;
    case Get_Event_ID:// 0xD528,
        size=sizeof(Event_comand_Get_Event_ID );// comand_Get_Event_ID;
        break;
    case Erase_screen:// 0xD602,
        size=sizeof(Event_comand_Erase_screen );// comand_Erase_screen;
        break;
    case Show_screen:// 0xD60C,
        size=sizeof(Event_comand_Show_screen );// comand_Show_screen;
        break;
    case Set_screen_tone:// 0xD616,
        size=sizeof(Event_comand_Set_screen_tone );// comand_Set_screen_tone;
        break;
    case Flash_screen:// 0xD620,
        size=sizeof(Event_comand_Flash_screen );// comand_Flash_screen;
        break;
    case Shake_screen:// 0xD62A,
        size=sizeof(Event_comand_Shake_screen );// comand_Shake_screen;
        break;
    case Pan_screen:// 0xD634,
        size=sizeof(Event_comand_Pan_screen );// comand_Pan_screen;
        break;
    case Weather_Effects:// 0xD63E,
        size=sizeof(Event_comand_Weather_Effects );// comand_Weather_Effects;
        break;
    case Show_Picture:// 0xD666,
        size=sizeof(Event_comand_Show_Picture );// comand_Show_Picture;
        break;
    case Move_Picture:// 0xD670,
        size=sizeof(Event_comand_Move_Picture );// comand_Move_Picture;
        break;
    case Erase_Picture:// 0xD67A,
        size=sizeof(Event_comand_Erase_Picture );// comand_Erase_Picture;
        break;
    case Show_Battle_Anim :// 0xD74A,
        size=sizeof(Event_comand_Show_Battle_Anim );// comand_Show_Battle_Anim;
        break;
    case Set_hero_opacity:// 0xD82E,
        size=sizeof(Event_comand_Set_hero_opacity );// comand_Set_hero_opacity;
        break;
    case Flash_event:// 0xD838,
        size=sizeof(Event_comand_Flash_event );// comand_Flash_event;
        break;
    case Move_event:// 0xD842,
        size=sizeof(Event_comand_Move_event );// comand_Move_event;
        break;
    case Wait:// 0xD912,
        size=sizeof(Event_comand_Wait );// comand_Wait;
        break;
    case Play_BGM:// 0xD976,
        size=sizeof(Event_comand_Play_BGM );// comand_Play_BGM;
        break;
    case Fade_out_BGM:// 0xDA00,
        size=sizeof(Event_comand_Fade_out_BGM );// comand_Fade_out_BGM;
        break;
    case Play_sound_effect:// 0xDA1E,

        size=sizeof(Event_comand_Play_SE );// comand_Play_SE;
        break;
    case Play_movie:// 0xDA28,
        size=sizeof(Event_comand_Play_movie );// comand_Play_movie;
        break;
    case Key_input:// 0xDA5A,
        size=sizeof(Event_comand_Key_input );// comand_Key_input;
        break;
    case Change_tile_set:// 0xDB3E	,
        size=sizeof(Event_comand_Change_tile );// comand_Change_tile;
        break;
    case Change_background:// 0xDB48,
        size=sizeof(Event_comand_Change_background );// comand_Change_background;
        break;
    case Change_single_tile:// 0xDB66,
        size=sizeof(Event_comand_Change_single_tile );// comand_Change_single_tile;
        break;
    case Set_teleport_location:// 0xDC22,
        size=sizeof(Event_comand_Set_teleport_location );// comand_Set_teleport_location;
        break;
    case Set_escape_location:// 0xDC36,
        size=sizeof(Event_comand_Set_escape_location );// comand_Set_escape_location;
        break;
    case Enable_system_menu:
        size=sizeof(Event_comand_Enable_system_menu );// comand_Enable_system_menu;
        break;
    case Conditional:// 0xDD6A,
        size=sizeof(Event_comand_Conditional );// comand_Conditional;
        break;
    case Label:// 0xDE4E,
        size=sizeof(Event_comand_Label );// comand_Label;
        break;
    case Go_to_label:// 0xDE58,
        size=sizeof(Event_comand_Go_to_label );// comand_Go_to_label;
        break;
    case Call_event:// 0xE02A,
        size=sizeof(Event_comand_Call_event );// comand_Call_event;
        break;
        default:
            size=sizeof((comands[i]));
        break;
    }

        comand_before_telepor.push_back(( Event_comand *)malloc(size));
        memcpy (comand_before_telepor[i],comands[i],size);
}

}
void General_data::read_database()
{
    static bool inited=false;

   if(!inited)//just one time
   {
    LDB_reader my_ldb;
    my_ldb.Load("RPG_RT.ldb", ldbdata);
    Players.data2 = ldbdata;
    lmt_reader my_lmt;

    E_state original_state;
    original_state.Event_Active=false;
    original_state.id_exe_actual=0;
    original_state.id_actual_active=false;
    original_state.Active_page= -2;//common event

    for(unsigned int i=0; i<ldbdata->Event.size();i++)
        GEvc_state.push_back(original_state);

    my_lmt.load("RPG_RT.lmt",&lmt);
  //  my_ldb.ShowInformation(&ldbdata);

   }
  // my_lmt.print(&lmt);
}
void General_data::add_swich(unsigned char i)
{
	world_fase.push_back(i);
}

bool General_data::is_on_the_inventory(int id)
{
unsigned int i;
for(i=0;i<Items.size();i++)
{
if(Items[i].id==id)
{
    return true;
}
}
return false;
}

int General_data::number_on_the_inventory(int id)
{
unsigned int i;
for(i=0;i<Items.size();i++)
{
if(Items[i].id==id)
{
    return Items[i].NOI;
}
}
return 0;
}

bool General_data::is_equal(int var,int number)
{
if(world_var[var-1]>=number)
return true;
return false;
}

bool General_data::state_swich( int number)
{
int unsigned real_id, position;
unsigned char state,temp;
number--;
real_id =(unsigned int)(number/8);
position= (number%8);
if(real_id<world_fase.size())
state=world_fase[real_id];
else
return(false);
temp=(1<<position);
return( state&temp );
}

void General_data::set_true_swich( int number)
{
number--;
int real_id, position;
real_id = (number/8);
position= (number%8);
world_fase[real_id]=(world_fase[real_id]|(1<<(position)));
}

void General_data::set_false_swich( int number)
{
number--;
int real_id, position;
real_id = (number/8);
position= (number%8);
world_fase[real_id]=(world_fase[real_id]&(!(1<<position)));
}
void General_data::clear_obj()
{
	Items.clear();
}
void General_data::clear_enemy()
{ unsigned int i;
    for(i=0;i<Enemys.size();i++)
    Enemys[i].Batler.dispose();
	Enemys.clear();
}
void General_data::load_group(int id)
{
    unsigned int i;
    int moster_id;
    clear_enemy();
   id--;
   cout<<" data "<<id<<endl;
    for(i=0;i<ldbdata->mosterpartys[id].Enemy_data.size();i++)
    {
        moster_id=ldbdata->mosterpartys[id].Enemy_data[i].Enemy_ID-1;
        cout<<" mid "<<moster_id<<endl;

        Enemy enemigo;
        enemigo.set_HP(ldbdata->mosters[moster_id].intMaxHP);
        enemigo.set_MaxHP(ldbdata->mosters[moster_id].intMaxHP);
        enemigo.set_MP(ldbdata->mosters[moster_id].intMaxMP);
        enemigo.set_MaxMP(ldbdata->mosters[moster_id].intMaxMP);
        enemigo.set_Attack(ldbdata->mosters[moster_id].intAttack);
        enemigo.set_Defense(ldbdata->mosters[moster_id].intDefense);
        enemigo.set_Speed(ldbdata->mosters[moster_id].intSpeed);
        enemigo.set_Spirit(ldbdata->mosters[moster_id].intMind);
        std::string system_string;
        system_string.clear();
        system_string.append("Monster/");
        system_string.append(ldbdata->mosters[moster_id].strGraphicfile);
        system_string.append(".png");
        (enemigo.Batler).setimg(system_string.c_str());
        (enemigo.Batler).setcols(1);
        (enemigo.Batler).setrows(1);
        (enemigo.Batler).x=ldbdata->mosterpartys[id].Enemy_data[i].X_position;
        (enemigo.Batler).y=ldbdata->mosterpartys[id].Enemy_data[i].Y_position;
        enemigo.set_name(ldbdata->mosters[moster_id].strName.c_str());
        add_enemy(enemigo);
    }

}

void General_data::add_enemy(Enemy Myplayer)
{
	Enemys.push_back(Myplayer);
}


Item General_data::get_item(int num)
{
	return (Items.at(num));
}

Item General_data::load_item(int item_id)
{
Item X;
if(item_id==0)
{
    X.set_name("");
    X.set_NOI(1);
    X.set_type(0);
    X.id = 0;
}
else{
    item_id--;
    X.set_name(ldbdata->items->at(item_id)->Name.c_str());
    X.set_NOI(1);
    X.set_type(ldbdata->items->at(item_id)->Type);
    X.id = 1;
    }return(X);
}

void General_data::add_item(Item Myitem)
{

	unsigned int i;
	int the_id;
	the_id=Myitem.id;
	for (i=0;i<Items.size();i++)
    	{
		if(the_id ==(Items.at(i)).id)
		{
			break;
		}
	}
	if (i<Items.size())//lo encontro
	{
		*((Items.at(i)).get_NOI())= *((Items.at(i)).get_NOI())+*Myitem.get_NOI();
    	}
	else
	{
	Items.push_back(Myitem);
	}
}

int General_data::get_num_items()
{
	return (Items.size());
}

void General_data::change_objets(int remove_add,int item_id,int cout)
{
    Item X;
    item_id--;
    X.set_name(ldbdata->items->at(item_id)->Name.c_str());
    X.set_NOI(cout);
    X.set_type(ldbdata->items->at(item_id)->Type);
    X.id = 1;

    if(remove_add)
        erase_item(item_id);
    else
        add_item(X);

}

void General_data::erase_item(int the_item)
{
	vector<Item>::iterator the_iterator;
	the_iterator=Items.begin();
	int i;
	for(i=0;i<the_item;i++)
	{
		the_iterator++;
	}
	Items.erase( the_iterator);
}
int* General_data::get_NOI(int num)
{
	return (((Items.at(num))).get_NOI());
}
unsigned char * General_data::get_type(int num)
{
	return (((Items.at(num))).get_type());
}
const char * General_data::get_item_name(int num)
{
	return (((Items.at(num))).get_name());
}
Animacion * General_data::get_item_anim(int num)
{
	return (((Items.at(num))).get_anim());
}
void General_data::set_Gold(int The_Gold)
{
	Gold=The_Gold;
}
int General_data::get_Gold()
{
	return (Gold);
}
