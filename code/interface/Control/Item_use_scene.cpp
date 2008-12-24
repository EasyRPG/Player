#include "Item_use_scene.h"
/*Item_use_scene::~Item_use_scene()
{
}
Item_use_scene::Item_use_scene()
{
}*/
void Item_use_scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{  int i;
   myteam=TheTeam;
   myaudio=theaudio;
   players.init(theaudio, run,0,((*myteam).get_size()-1),184,240,136,0,124,48);
   players.init_curXY(55,5); //ya eran muchos comandos
   players.visible=true;
   itemwin.init(136,30,0,0);
   itemwin2.init(136,30,0,30);

   char stringBuffer[255];
     i=(*myteam).select;
    itemwin.add_text(((*myteam).get_item_name(i)) ,5,5);
    sprintf(stringBuffer, "Objetos prop.  %d", (*(*myteam).get_NOI(i)));
   itemwin2.add_text(stringBuffer,5,5);
   running=  run;
   NScene=TheScene;

int space=60;

for(i=0;i<(*myteam).get_size();i++)
{
   players.add_sprite(((*myteam).get_faceset(i)),5,5+(i*space));
   players.add_text(((*myteam).get_name(i)),55,2+(i*space));

   sprintf(stringBuffer, "Level %d ", (*(*myteam).get_Level(i)));
   players.add_text(stringBuffer,55,20+(i*space));
   players.add_text("Normal",55,37+(i*space));
   sprintf(stringBuffer, "Hp %d / %d", (*(*myteam).get_HP(i)), (*(*myteam).get_MaxHP(i)));
   players.add_text(stringBuffer,110,20+(i*space));
   sprintf(stringBuffer, "Mp %d / %d", (*(*myteam).get_MP(i)), (*(*myteam).get_MaxMP(i)));
   players.add_text(stringBuffer,110,37+(i*space));
}
 retardo =0;
}

void Item_use_scene::update(SDL_Surface* Screen)
{
if(retardo==0)
{SDL_FillRect(Screen, NULL, 0x0);// Clear screen
 itemwin.draw(Screen);
 players.draw(Screen);
 itemwin2.draw(Screen);
}
retardo++;
   if(retardo==5)
   {
   itemwin.draw(Screen);
    players.draw(Screen);
     itemwin2.draw(Screen);
   retardo=1;
   }
}

void Item_use_scene::action()
{int i;
   char stringBuffer[255];
  i=(*myteam).select;
  (*(*myteam).get_NOI(i))=(*(*myteam).get_NOI(i))-1;

  if((*(*myteam).get_NOI(i)) !=0)
  {
  itemwin2.dispose();
  itemwin2.init(136,30,0,30);
  sprintf(stringBuffer, "Objetos prop.  %d", (*(*myteam).get_NOI(i)));
  itemwin2.add_text(stringBuffer,5,5);
players.restarmenu();
  }else{
   (*myteam).erase_item(i);
   * NScene=5;
  }


}

void Item_use_scene::updatekey() {
if(players.visible)
{
players.updatekey();
if(players.desition())
action();
}

 if (Key_press_and_realsed(LMK_X ))//retorna alos objetos
        { (*myaudio).soundload("../Sound/Cansel2.wav");* NScene=5; }
     }
void Item_use_scene::dispose() {
   players.dispose();
   itemwin.dispose();
   itemwin2.dispose();
}
