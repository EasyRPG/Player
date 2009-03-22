#include "Event_management.h"

General_data::General_data()
{
    running = true;
    TheScene = 0;
}

void General_data::read_database()
{
    static bool inited=false;

   if(!inited)//just one time
   {
    LDB_reader my_ldb;
    my_ldb.Load("RPG_RT.ldb",&data2);
    Players.data2=&data2;
    lmt_reader my_lmt;

    E_state original_state;
    original_state.Event_Active=false;
    original_state.id_exe_actual=0;
    original_state.id_actual_active=false;
    original_state.Active_page= -2;//common event

    for(unsigned int i=0; i<data2.Event.size();i++)
        GEvc_state.push_back(original_state);

    my_lmt.load("RPG_RT.lmt",&lmt);
  //  my_ldb.ShowInformation(&data2);

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
    for(i=0;i<data2.mosterpartys[id].Enemy_data.size();i++)
    {
        moster_id=data2.mosterpartys[id].Enemy_data[i].Enemy_ID-1;
        cout<<" mid "<<moster_id<<endl;

        Enemy enemigo;
        enemigo.set_HP(data2.mosters[moster_id].intMaxHP);
        enemigo.set_MaxHP(data2.mosters[moster_id].intMaxHP);
        enemigo.set_MP(data2.mosters[moster_id].intMaxMP);
        enemigo.set_MaxMP(data2.mosters[moster_id].intMaxMP);
        enemigo.set_Attack(data2.mosters[moster_id].intAttack);
        enemigo.set_Defense(data2.mosters[moster_id].intDefense);
        enemigo.set_Speed(data2.mosters[moster_id].intSpeed);
        enemigo.set_Spirit(data2.mosters[moster_id].intMind);
        std::string system_string;
        system_string.clear();
        system_string.append("Monster/");
        system_string.append(data2.mosters[moster_id].strGraphicfile);
        system_string.append(".png");
        (enemigo.Batler).setimg(system_string.c_str());
        (enemigo.Batler).setcols(1);
        (enemigo.Batler).setrows(1);
        (enemigo.Batler).x=data2.mosterpartys[id].Enemy_data[i].X_position;
        (enemigo.Batler).y=data2.mosterpartys[id].Enemy_data[i].Y_position;
        enemigo.set_name(data2.mosters[moster_id].strName.c_str());
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
    X.set_name(data2.items[item_id].Name.c_str());
    X.set_NOI(1);
    X.set_type(data2.items[item_id].Type);
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
    X.set_name(data2.items[item_id].Name.c_str());
    X.set_NOI(cout);
    X.set_type(data2.items[item_id].Type);
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
