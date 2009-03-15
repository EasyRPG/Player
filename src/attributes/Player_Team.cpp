/*Player_Team.cpp, Player_Team routines.
    Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "Player_Team.h"



bool Player_Team::is_on_the_team(int id)
{
unsigned int i;
for(i=0;i<Players.size();i++)
{
if(Players[i].id==id)
{
    return true;
}
}
return false;
}

void Player_Team::clear_team()
{
    unsigned int i;
    for(i=0;i<Players.size();i++)
    {
    Players[i].charset.dispose();
    Players[i].Face.dispose();
    ((get_Weapon(i))->get_anim())->dispose();//las armas no debrian tener anim, solo un id
    }
	Players.clear();
}
void Player_Team::add_player(Player Myplayer)
{
    if(!is_on_the_team(Myplayer.id))
	Players.push_back(Myplayer);
}

int Player_Team::get_size()
{
	return (Players.size());
}

int Player_Team::get_xp_for_level(int Level,int star_exp, int addIncrease, int correction )
{
float exp_perlevel;
float aditional;

int i;
int result =0;
exp_perlevel= star_exp;

 aditional = 1.5 +(addIncrease* 0.01);

for(i=Level; i>0;i--)
{
        result= result+ correction + exp_perlevel;
        exp_perlevel = exp_perlevel * aditional;
        aditional = (((Level * 0.002) + 0.8) * (aditional - 1) +1);
}

return((int)result);
}

void Player_Team::change_name(int Hero_ID,const char * name)
{
    unsigned i;
    if(!is_on_the_team(Hero_ID)) // si no esta retorna
    return;
    for(i=0;i<Players.size();i++)
    {
        if(Players[i].id==Hero_ID)
        {
            break;
        }
    }
    Players[i].set_name(name);
}


void Player_Team::change_face(int Hero_ID,const char * name,int id)
{
    unsigned i;
    if(!is_on_the_team(Hero_ID)) // si no esta retorna
    return;
    for(i=0;i<Players.size();i++)
    {
        if(Players[i].id==Hero_ID)
        {
            break;
        }
    }
    Players[i].Face.setimg(name);
    Players[i].Face.init_Faceset(0,0,id);
}
void Player_Team::change_graphic(int Hero_ID,const char * name,int id)
{
    unsigned i;
    if(!is_on_the_team(Hero_ID)) // si no esta retorna
    return;
    for(i=0;i<Players.size();i++)
    {
        if(Players[i].id==Hero_ID)
        {
            break;
        }
    }
    Players[i].charset.setimg(name,id);
}
void Player_Team::change_class(int Hero_ID,const char * name)
{
    unsigned i;
    if(!is_on_the_team(Hero_ID)) // si no esta retorna
    return;
    for(i=0;i<Players.size();i++)
    {
        if(Players[i].id==Hero_ID)
        {
            break;
        }
    }
    Players[i].set_job(name);
}


void Player_Team::change_level(int add_remove,int Hero_ID, int count)
{
   unsigned int i,j=0;

    if(!is_on_the_team(Hero_ID)) // si no esta retorna
    return;

    for(i=0;i<Players.size();i++)
    {
        if(Players[i].id==Hero_ID)
        {
            j=i;
            break;
        }
    }
    stcHero * actual_hero;
    actual_hero= &(data2->heros[Hero_ID-1]);
     i= Players[j].get_Level();

    if(add_remove)
    {
        i-=count;
    }
    else
    {
        i+=count;
    }
    Players[j].set_HP(actual_hero->vc_sh_Hp[i]);
    Players[j].set_MaxHP(actual_hero->vc_sh_Hp[i]);
    Players[j].set_MP(actual_hero->vc_sh_Mp[i]);
    Players[j].set_MaxMP(actual_hero->vc_sh_Mp[i]);
    Players[j].set_Heal(0);
    Players[j].set_Attack(actual_hero->vc_sh_Attack[i]);
    Players[j].set_Defense(actual_hero->vc_sh_Defense[i]);
    Players[j].set_Speed(actual_hero->vc_sh_Agility[i]);
    Players[j].set_Spirit(actual_hero->vc_sh_Mind[i]);
    Players[j].set_Level(i);
    Players[j].set_Exp(get_xp_for_level(i-1,actual_hero->intEXPBaseline,actual_hero->intEXPAdditional,actual_hero->intEXPCorrection ));
    Players[j].set_MaxExp(get_xp_for_level(i,actual_hero->intEXPBaseline,actual_hero->intEXPAdditional,actual_hero->intEXPCorrection ));
}


void Player_Team::change_stats(int add_remove,int Hero_ID, int count,int type)
{
   unsigned int i,j=0;
    if(!is_on_the_team(Hero_ID)) // si no esta retorna
    return;

    for(i=0;i<Players.size();i++)
    {
        if(Players[i].id==Hero_ID)
        {
            j=i;
            break;
        }
    }

    if(type==0)//pv
    {
        if(add_remove)
        {
            i= Players[j].get_MaxHP();
            Players[j].set_MaxHP(i-count);
        }
        else
        {
            i= Players[j].get_MaxHP();
            Players[j].set_MaxHP(i+count);
        }
    }

    if(type==1)//pm
    {
        if(add_remove)
        {
            i= Players[j].get_MaxMP();
            Players[j].set_MaxMP(i-count);
        }
        else
        {
            i= Players[j].get_MaxMP();
            Players[j].set_MaxMP(i+count);
        }
    }

    if(type==2)//ataque
    {
        if(add_remove)
        {
            i= Players[j].get_Attack();
            Players[j].set_Attack(i-count);
        }
        else
        {
            i= Players[j].get_Attack();
            Players[j].set_Attack(i+count);
        }
    }

    if(type==3)//defensa
    {
       if(add_remove)
        {
            i= Players[j].get_Defense();
            Players[j].set_Defense(i-count);
        }
        else
        {
            i= Players[j].get_Defense();
            Players[j].set_Defense(i+count);
        }
    }

    if(type==4)//espiritu
    {
        if(add_remove)
        {
            i= Players[j].get_Spirit();
            Players[j].set_Spirit(i-count);
        }
        else
        {
            i= Players[j].get_Spirit();
            Players[j].set_Spirit(i+count);
        }
    }

    if(type==5)//agilidad
    {
        if(add_remove)
        {
            i= Players[j].get_Speed();
            Players[j].set_Speed(i-count);
        }
        else
        {
            i= Players[j].get_Speed();
            Players[j].set_Speed(i+count);
        }
    }
}

void Player_Team::change_exp(int add_remove,int Hero_ID, int count)
{
   unsigned int i,j=0;

    if(!is_on_the_team(Hero_ID)) // si no esta retorna
    return;

    for(i=0;i<Players.size();i++)
    {
        if(Players[i].id==Hero_ID)
        {
            j=i;
            break;
        }
    }


    if(add_remove)
    {
        i= Players[j].get_Exp();
        Players[j].set_Exp(i-count);
    }
    else
    {
        i= Players[j].get_Exp();
        Players[j].set_Exp(i+count);
    }

}

CActor * Player_Team::get_chara(int num)
{
	return (((Players.at(num))).get_chara());
}
Faceset * Player_Team::get_faceset(int num)
{
	return (((Players.at(num))).get_faceset());
}
const char * Player_Team::get_name(int num)
{
	return (((Players.at(num))).get_name());
}
const char * Player_Team::get_job(int num)
{
	return (((Players.at(num))).get_job());
}
int Player_Team::get_HP(int num)
{
	return (((Players.at(num))).get_HP());
}

void Player_Team::set_HP(int num,int hp)
{
	return (((Players.at(num))).set_HP(hp));
}

int Player_Team::get_MaxHP(int num)
{
	return (((Players.at(num))).get_MaxHP());
}
int Player_Team::get_MP(int num)
{
	return (((Players.at(num))).get_MP());
}
int Player_Team::get_MaxMP(int num)
{
	return (((Players.at(num))).get_MaxMP());
}

int Player_Team::get_Heal(int num)
{
	return (((Players.at(num))).get_Heal());
}
int Player_Team::get_Attack(int num)
{
	return (((Players.at(num))).get_Attack());
}
int Player_Team::get_Defense(int num)
{
	return (((Players.at(num))).get_Defense());
}
int Player_Team::get_Speed(int num)
{
	return (((Players.at(num))).get_Speed());
}

int Player_Team::get_Spirit(int num)
{
	return (((Players.at(num))).get_Spirit());
}
int Player_Team::get_Level(int num)
{
	return (((Players.at(num))).get_Level());
}
int Player_Team::get_Exp(int num)
{
	return (((Players.at(num))).get_Exp());
}
int Player_Team::get_MaxExp(int num)
{
	return (((Players.at(num))).get_MaxExp());
}

void Player_Team::change_players(int remove_add,int id)
{
unsigned int i;

    if(remove_add)
    {
        for(i=0;i<Players.size();i++)
        {
            if(Players[i].id==id)
            {
                Players.erase(Players.begin()+i);
            }
        }
    }
    else
    {
        stcHero * actual_hero;
        actual_hero= &(data2->heros[id-1]);
        add_player(get_hero(actual_hero, id));
    }
}

void Player_Team::change_MP(int Add,int Hero_id,int count)
{
   int i;
   unsigned int  j;

    if(!is_on_the_team(Hero_id)) // si no esta retorna
    return;

    for(j=0;j<Players.size();j++)
    {
        if(Players[j].id==Hero_id)
        {
            break;
        }
    }
    if(Add)
    {
        i= Players[j].get_MP()-count;
        if(i<1)
        i=0;
        Players[j].set_MP(i);
    }
    else
    {
        i= Players[j].get_MP()+count;
        if(i>Players[j].get_MaxMP())
        i=Players[j].get_MaxMP();
        Players[j].set_MP(i);
    }
}


void Player_Team::Full_Recovery(int Hero_id)
{
   unsigned int  j;

    if(!is_on_the_team(Hero_id)) // si no esta retorna
    return;

    for(j=0;j<Players.size();j++)
    {
        if(Players[j].id==Hero_id)
        {
            break;
        }
    }
    Players[j].set_HP(Players[j].get_MaxHP());
    Players[j].set_MP(Players[j].get_MaxMP());

}
void Player_Team::change_HP(int Add,int Hero_id,int count)
{
   int i;
   unsigned int  j;

    if(!is_on_the_team(Hero_id)) // si no esta retorna
    return;

    for(j=0;j<Players.size();j++)
    {
        if(Players[j].id==Hero_id)
        {
            break;
        }
    }

    if(Add)
    {
        i= Players[j].get_HP()-count;
        if(i<1)
        i=1;
        Players[j].set_HP(i);
    }
    else
    {
        i= Players[j].get_HP()+count;
        if(i>Players[j].get_MaxHP())
        i=Players[j].get_MaxHP();
        Players[j].set_HP(i);
    }
}
void Player_Team::change_skills(int Learn,int Hero_id,int skill_id)
{
    Skill Veneno;
    unsigned int i,j=0;
    if(!is_on_the_team(Hero_id)) // si no esta retorna
    return;

        for(i=0;i<Players.size();i++)
        {
            if(Players[i].id==Hero_id)
            {
              j=i;
              break;
            }
        }

    if(Learn)
    {   //remove
        //serch for it
        for(i=0;i<Players[j].Skills.size();i++)
        {
            if(Players[j].Skills[i].id==(skill_id-1))
            {
                Players[j].Skills.erase(Players[j].Skills.begin()+i);
              break;
            }
        }
    }
    else
    {
        if(!hero_has_skill(j,skill_id-1))
        {
            skill_id=skill_id-1;
            Veneno.id=skill_id;
            Veneno.set_name(data2->skill[skill_id].strName.c_str());
            Veneno.set_damange(data2->skill[skill_id].intBasevalue);
            Veneno.set_level_req(1);
            Veneno.set_mp_price(data2->skill[skill_id].intCost);
            Players[j].add_skill(Veneno);
        }
    }
}

bool Player_Team::hero_has_skill(int hero_static_id,int id)
{
    unsigned int i;
    printf("dinero %d %d",hero_static_id,id);
        for(i=0;i<Players[hero_static_id].Skills.size();i++)
        {
            if(Players[hero_static_id].Skills[i].id==id)
            {
                return(true);
            }
        }
        return(false);
}

Player Player_Team::get_hero(stcHero * actual_hero,int id)
{
    int start_level,id_skill;
    unsigned j;
    string system_string;

    Player Alex;
    Alex.id=id;
    Alex.set_name(actual_hero->strName.c_str());
    Alex.set_job(actual_hero->strClass.c_str());
    start_level=actual_hero->intStartlevel;

    Faceset AlexeFase;
    system_string.clear();
    system_string.append("FaceSet/");
    system_string.append(actual_hero->strFacegraphic.c_str());
    system_string.append(".png");
    AlexeFase.setimg(system_string.c_str());
    AlexeFase.init_Faceset(0, 0, actual_hero->intFaceindex);
    Alex.set_faceset(AlexeFase);

    CActor AlexChara;
    system_string.clear();
    system_string.append("CharSet/");
    system_string.append(actual_hero->strGraphicfile);
    system_string.append(".png");
    AlexChara.init_Chara();
    AlexChara.setimg((char *)system_string.c_str(),actual_hero->intGraphicindex);

    Alex.set_chara(AlexChara);



    Alex.set_HP(actual_hero->vc_sh_Hp[start_level]);
    Alex.set_MaxHP(actual_hero->vc_sh_Hp[start_level]);
    Alex.set_MP(actual_hero->vc_sh_Mp[start_level]);
    Alex.set_MaxMP(actual_hero->vc_sh_Mp[start_level]);
    Alex.set_Heal(0);
    Alex.set_Attack(actual_hero->vc_sh_Attack[start_level]);
    Alex.set_Defense(actual_hero->vc_sh_Defense[start_level]);
    Alex.set_Speed(actual_hero->vc_sh_Agility[start_level]);
    Alex.set_Spirit(actual_hero->vc_sh_Mind[start_level]);
    Alex.set_Level(start_level);
    Alex.set_Exp(get_xp_for_level(start_level-1,actual_hero->intEXPBaseline,actual_hero->intEXPAdditional,actual_hero->intEXPCorrection ));
    Alex.set_MaxExp(get_xp_for_level(start_level,actual_hero->intEXPBaseline,actual_hero->intEXPAdditional,actual_hero->intEXPCorrection ));
//las habilidades del alex que hueva.....
   Skill Veneno;
Alex.Skills.clear();
for (j=0;j<(actual_hero->skills.size());j++)
{
    id_skill=actual_hero->skills[j].Spell_ID-1;
    Veneno.id=id_skill;
    Veneno.set_name(data2->skill[id_skill].strName.c_str());
    Veneno.set_damange(data2->skill[id_skill].intBasevalue);
    Veneno.set_level_req(1);
    Veneno.set_mp_price(data2->skill[id_skill].intCost);
    Alex.add_skill(Veneno);
}


    Item X;

    X.set_name(data2->items[actual_hero->sh_Weapon].Name.c_str());
    X.set_NOI(1);
    X.set_type(data2->items[actual_hero->sh_Weapon].Type);
    X.id = 1;

    Alex.set_Weapon(X);

    X.set_name(data2->items[actual_hero->sh_Shield].Name.c_str());
    X.set_NOI(1);
    X.set_type(data2->items[actual_hero->sh_Shield].Type);
    X.id = 1;

    Alex.set_Shield(X);

    X.set_name(data2->items[actual_hero->sh_Armor].Name.c_str());
    X.set_NOI(1);
    X.set_type(data2->items[actual_hero->sh_Armor].Type);
    X.id = 1;


    Alex.set_Armor(X);

    X.set_name(data2->items[actual_hero->sh_Head].Name.c_str());
    X.set_NOI(1);
    X.set_type(data2->items[actual_hero->sh_Head].Type);
    X.id = 1;

    Alex.set_Helmet(X);

    X.set_name(data2->items[actual_hero->sh_Accessory].Name.c_str());
    X.set_NOI(1);
    X.set_type(data2->items[actual_hero->sh_Accessory].Type);
    X.id = 1;


    Alex.set_Accessory(X);

    Alex.set_Weapon_type(4);
    Alex.set_Shield_type(5);
    Alex.set_Armor_type(6);
    Alex.set_Helmet_type(7);
    Alex.set_Accessory_type(8);
    return(Alex);
}

int Player_Team::get_skill_size(int num)
{
	return (((Players.at(num))).get_skill_size());
}
const char * Player_Team::get_skill_name(int nump,int num)
{
	return (((Players.at(nump))).get_skill_name(num));
}
int * Player_Team::get_skill_mp_price(int nump,int num)
{
	return (((Players.at(nump))).get_skill_mp_price(num));
}
int * Player_Team::get_skill_damange(int nump,int num)
{
	return (((Players.at(nump))).get_skill_damange(num));
}
int * Player_Team::get_skill_level_req(int nump,int num)
{
	return (((Players.at(nump))).get_skill_level_req(num));
}

Animacion * Player_Team::get_skill_get_anim(int nump,int num)
{
	return (((Players.at(nump))).get_skill_get_anim(num));
}
Animacion * Player_Team::get_Weapon_Anim(int nump)
{
	return (((Players.at(nump))).get_Weapon_Anim());
}

Item* Player_Team::get_Weapon(int nump)
{
	return ((Players.at(nump)).get_Weapon());
}
Item* Player_Team::get_Shield(int nump)
{
	return ((Players.at(nump)).get_Shield());
}
Item* Player_Team::get_Armor(int nump)
{
	return ((Players.at(nump)).get_Armor());
}
Item* Player_Team::get_Helmet(int nump)
{
	return ((Players.at(nump)).get_Helmet());
}
Item* Player_Team::get_Accessory(int nump)
{
	return ((Players.at(nump)).get_Accessory());
}

void Player_Team::set_Weapon(int nump,Item The_Weapon)
{
	(Players.at(nump)).set_Weapon(The_Weapon);
}
void Player_Team::set_Shield(int nump,Item The_Shield)
{
	(Players.at(nump)).set_Shield(The_Shield);
}
void Player_Team::set_Armor(int nump,Item The_Armor)
{
	(Players.at(nump)).set_Armor(The_Armor);
}
void Player_Team::set_Helmet(int nump,Item The_Helmet)
{
	(Players.at(nump)).set_Helmet(The_Helmet);
}
void Player_Team::set_Accessory(int nump,Item The_Accessory)
{
	(Players.at(nump)).set_Accessory(The_Accessory);
}


unsigned char* Player_Team::get_Weapon_type(int nump)
{
	return ( (Players.at(nump)).get_Weapon_type());
}
unsigned char* Player_Team::get_Shield_type(int nump)
{
	return ( (Players.at(nump)).get_Shield_type());
}
unsigned char* Player_Team::get_Armor_type(int nump)
{
	return ( (Players.at(nump)).get_Armor_type());
}
unsigned char* Player_Team::get_Helmet_type(int nump)
{
	return ( (Players.at(nump)).get_Helmet_type());
}
unsigned char* Player_Team::get_Accessory_type(int nump)
{
	return ( (Players.at(nump)).get_Accessory_type());
}

void Player_Team::set_Weapon_type(int nump, unsigned char The_Weapon_type)
{
	(Players.at(nump)).set_Weapon_type(The_Weapon_type);
}
void Player_Team::set_Shield_type(int nump, unsigned char The_Shield_type)
{
	(Players.at(nump)).set_Shield_type(The_Shield_type);
}
void Player_Team::set_Armor_type(int nump, unsigned char The_Armor_type)
{
	(Players.at(nump)).set_Armor_type(The_Armor_type);
}
void Player_Team::set_Helmet_type(int nump, unsigned char The_Helmet_type)
{
	(Players.at(nump)).set_Helmet_type(The_Helmet_type);
}
void Player_Team::set_Accessory_type(int nump, unsigned char The_Accessory_type)
{
	(Players.at(nump)).set_Accessory_type(The_Accessory_type);
}
