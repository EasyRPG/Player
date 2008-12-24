#include "Batle_scene.h"
/*
Batle_scene::~Batle_scene()
{

}

Batle_scene::Batle_scene()
{

}
*/

void Batle_scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{
myteam=TheTeam;
myaudio=theaudio;
the_run=run;
(*myaudio).musicload("../Music/Battle2.mid");
title.x=0;
title.y=0;
title.setimg("../Backdrop/Grass.png");
Window_text.init(320, 80,0, 160);
Window_text.visible=false;
update_window_stats();
// menu_os.init( theaudio, run, 0,4, 96, 80, 96, 160);
// menu_os.visible=false;
menu.init( theaudio, run, 0,4, 96, 80, 0, 160);
str_Vector.push_back("Atacar ");
     str_Vector.push_back("Habilidades ");
     str_Vector.push_back("Objetos ");
     str_Vector.push_back("Defender ");
     str_Vector.push_back("Huir ");
     menu.setComands(& str_Vector);
     running=  run;
     NScene=TheScene;
     MC comando;
     int i;
     for(i=0;i<(*myteam).get_size();i++) //tantos comandos como jugadores.
        Comands.push_back(comando);
     state=0;
     Nmenu_used=0;
player_in_turn=0;
moster_in_turn=0;
turnosp=0;
update_window_mosterselect();
moster_select.visible=false;
}
void Batle_scene::update_window_stats()
{
 window.init(myaudio,the_run,0,3,224,80,96,160,214,16 );
 int i=0;
 char stringBuffer[255];
 for(i=0;i<(*myteam).get_size();i++)
  {
   sprintf(stringBuffer, "Hp %d / %d  Mp %d ", (*(*myteam).get_HP(i)), (*(*myteam).get_MaxHP(i)),(*(*myteam).get_MP(i)));
   window.add_text(stringBuffer,110, 5+(i*16));
   window.add_text(((*myteam).get_name(i)),10,5+(i*16));
  if((*(*myteam).get_HP(i))>0)
   window.add_text("Normal",60, 5+(i*16));
  else
   window.add_text("Muerto",60, 5+(i*16));
   }

}

void Batle_scene::windowtext_showdamange(bool type,int atak,int ataked,int damange)
{
char stringBuffer[255];
sprintf(stringBuffer, "%d HP perdidos ",damange );
Window_text.init(320, 80,0, 160);

if(type)//si son los players
{
Window_text.add_text(((*myteam).get_name(atak)),5,5);//nombre heroe
Window_text.add_text("ataca al enemigo",70, 5);
Window_text.add_text((((*myteam).Enemys.at(ataked)).get_name()),5, 25);//nombre moustruo
Window_text.add_text(stringBuffer,70, 25);
}else
{
Window_text.add_text((((*myteam).Enemys.at(atak)).get_name()),5, 5);//nombre moustruo
Window_text.add_text("ataca ",70, 5);
Window_text.add_text(((*myteam).get_name(ataked)),5,25);//nombre heroe
Window_text.add_text(stringBuffer,70, 25);
}




}

void Batle_scene::update_window_mosterselect()
{ int i,j,k=0;
 j=(*myteam).Enemys.size();

for(i=0;i<j;i++)   //dibuja todos los moster
 {
 if( (*((*myteam).Enemys.at(i)).get_HP())>0)//cambiar por arreglo
 {
 str_Vector2.push_back(((*myteam).Enemys.at(i)).get_name());
 k++;
 }
}
  moster_select.init( myaudio, the_run, 0,k-1, 96, 80, 0, 160);
 moster_select.setComands(& str_Vector2);

}


void Batle_scene::update(SDL_Surface* Screen)
{ int i,j;
SDL_FillRect(Screen, NULL, 0x0);// Clear screen
  j=(*myteam).Enemys.size();
  title.draw(Screen);
  window.draw(Screen);
  menu.draw(Screen);
  moster_select.draw(Screen);
  Window_text.draw(Screen);
  for(i=0;i<j;i++)   //dibuja todos los moster
   (((*myteam).Enemys.at(i)).Batler).draw(Screen);
  if(state==1) //si le toca alos heroes
   atack(Screen,player_in_turn,Comands.at(player_in_turn).selected_moster);
  if(state==2)//si le toca a los moustruos
   atacked(moster_in_turn);

}

void Batle_scene::win()
{
unsigned int i;
int k=0;
for (i=0;i<((*myteam).Enemys).size();i++)
if( (*((*myteam).Enemys.at(i)).get_HP())==0)//cambiar por arreglo
{
(((*myteam).Enemys.at(i)).Batler).visible=false;//haz que ya no se vea
 k++;
}
if (k==(*myteam).get_size())//si todos los enemigos muetros
    * NScene=1;//sal al mapa
}
void Batle_scene::lose()
{
int i,k=0;

for (i=0;i<(*myteam).get_size();i++)
if((*(*myteam).get_HP(i))==0)
  {   k++;
  }
if (k==(*myteam).get_size())//si todos los heroes muetros
    * NScene=3;//game over
}




void Batle_scene::atack(SDL_Surface* Screen,int nperso,int enemy)
{ int damange;
while((*(((*myteam).Enemys.at(enemy)).get_HP()))==0)//si  esta muerto el elgido
{enemy++;//elige otro
enemy=(enemy%((*myteam).Enemys).size());
}

(*((*myteam).get_Weapon_Anim(nperso))) .x=(((*myteam).Enemys.at(enemy)).Batler).x-((((*myteam).Enemys.at(enemy)).Batler).getw())/2;
(*((*myteam).get_Weapon_Anim(nperso))) .y=(((*myteam).Enemys.at(enemy)).Batler).y-((((*myteam).Enemys.at(enemy)).Batler).geth())/2;
(*((*myteam).get_Weapon_Anim(nperso))) .draw(Screen);


if((*((*myteam).get_Weapon_Anim(nperso))) .endanim)//si termina le atake
{
   (*((*myteam).get_Weapon_Anim(nperso))) .reset();
      damange=(*((*myteam).get_Attack(nperso)));
   (*(((*myteam).Enemys.at(enemy)).get_HP()))=(*(((*myteam).Enemys.at(enemy)).get_HP()))-damange;
Window_text.dispose();
   windowtext_showdamange(true,nperso,enemy,damange);

   if((*(((*myteam).Enemys.at(enemy)).get_HP()))<0)
   (*(((*myteam).Enemys.at(enemy)).get_HP()))=0;
   if((turnosp+1)<(*myteam).get_size())
   {
     player_in_turn++;//deveria ser una tabla
     turnosp++;
     win();
   }
   else
  {win();
   state=2;
  }//les toca a los moustruos

}
}
void Batle_scene::atacked(int enemy)
{
int i,j;
static   int posxt =title.x,flag =0,timer =0,moves =0;
static bool finish =false;
if((((*myteam).Enemys.at(enemy)).Batler).visible)//si esta vivo el enemigo
{
timer++;
if(timer==4)
{
 flag++;timer=0;
if(flag%2)
{
title.x=posxt+20;
   j=(*myteam).Enemys.size();
for(i=0;i<j;i++)
(((*myteam).Enemys.at(i)).Batler).x=(((*myteam).Enemys.at(i)).Batler).x+20;
}
else
{
flag=0;moves++;
timer=0;
title.x=posxt-20;
   j=(*myteam).Enemys.size();
for(i=0;i<j;i++)
(((*myteam).Enemys.at(i)).Batler).x=(((*myteam).Enemys.at(i)).Batler).x-20;
}
}
if(moves==10)
{
moves=11;
flag=0;
timer=10;
title.x=posxt;/////////////////////////restaurado de posiciones
j=(*myteam).Enemys.size();


int damange;
///////////////////////////////////////////elecion de player
int k =(rand()%(*myteam).get_size());//eleccion al azar
while((*(*myteam).get_HP(k))==0)//si  esta muerto el elgido
{k++;//elige otro
k=(k%(*myteam).get_size());
}
///////////////////////////////////////////////////////////////

damange=*(((*myteam).Enemys.at(enemy)).get_Attack()); //calculo de daño
 (*(*myteam).get_HP(k))=(*(*myteam).get_HP(k))-damange;
if((*(*myteam).get_HP(k))<0)
(*(*myteam).get_HP(k))=0;
//////////////////////////////////////////////////////////////////////////
lose();
Window_text.dispose();
windowtext_showdamange(false,enemy ,k,damange);
  finish=true;

}


if (finish)
{
if(timer==120)
{
moves=0;
flag=0;
timer=0;
finish=false;
j=(*myteam).Enemys.size();
    if(moster_in_turn+1<j)
   {moster_in_turn++;}
   else
   { state=0;
   Give_turn();//le toca a los comandos
   }
}

}

}else  //si el enemigo esta muerto
{

moves=0;
flag=0;
timer=0;
finish=false;
j=(*myteam).Enemys.size();
if(moster_in_turn+1<j)//si aun hay moustruos
{moster_in_turn++;}//que le toque a otro
else
{
state=0;//reinicimaos la batalla
Give_turn();//le toca a los comandos
}
}
}
void Batle_scene::Give_turn()
{moster_select.dispose();
 update_window_mosterselect();
 window.dispose();
 update_window_stats();
 menu.restarmenu();
 menu.visible=true;
 Window_text.visible=false;
 moster_select.visible=false;
 window.visible_window=true;
 Nmenu_used=0;
}
void Batle_scene::action_mosterselect()
{
 int i,j;
     j=(*myteam).Enemys.size();
    for(i=0;i<j;i++)
   if(moster_select.getindexY()==i)
   {  Comands.at(Nmenu_used).selected_moster=i;
      Nmenu_used++;
    moster_select.restarmenu();
    moster_select.visible=false;
    menu.visible=true;
   }

      if(Nmenu_used==(*myteam).get_size())//ya todos eligieron
    { state=1;
    menu.visible=false;
    Window_text.visible=true;
    window.visible_window=false;
    player_in_turn=0;//no heroes a husado ningun turno
    moster_in_turn=0;//los moustruos tampo han usad
    turnosp=0;
    }
}

void Batle_scene::action()
{
int i;
    //Comands.at(num).des2
if(menu.visible)
  {
   for(i=0;i<4;i++)
   if(menu.getindexY()==i)
   {Comands.at(Nmenu_used).des1=i;

    menu.restarmenu();
    menu.visible=false;
    moster_select.visible=true;
   }


   if(menu.getindexY()==4)
    {
   // state=1;
    * NScene=1;
     }
   }
}

void Batle_scene::updatekey()
{


if(Nmenu_used<(*myteam).get_size())//si aun no han elegido todos
{

if(moster_select.visible)
{
moster_select.updatekey();
if(moster_select.desition())
 action_mosterselect();
}
if(menu.visible)
{
menu.updatekey();
if(window.visible!=true)//que se vea que perso elige
window.visible=true;
window.set_curY((16*Nmenu_used) +5);//posicionado en el perso
if(menu.desition())
     action();
}


}
}

void Batle_scene::dispose()
{

title.dispose();
window.dispose();
(*myteam).clear_enemy();
(*myaudio).stopmusic();
menu.dispose();

}

