/* main.cpp, EasyRPG player main file.
Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.*/


#include "code/tools/tools.h"
#include "code/sprites/sprite.h"
#include "code/tools/key.h"
#include "code/tools/font.h"
#include "code/tools/audio.h"
#include "code/tools/math-sll.h"
#include "code/tools/Timer.h"
#include "code/tools/CDeltaTime.h"
#include <vector>
#include <string>
#include "code/attributes/skill.h"
#include "code/attributes/item.h"
#include "code/attributes/Enemy.h"
#include "code/attributes/Player.h"

#include "code/attributes/CActor.h"
#include "code/control/scene.h"
#include "code/control/Batle_scene.h"
#include "code/control/GameOver_scene.h"
#include "code/control/Map_scene.h"
#include "code/control/Title_scene.h"

#include "code/interface/Control/Equip_Menu_scene.h"
#include "code/interface/Control/Item_use_scene.h"
#include "code/interface/Control/Main_Menu_scene.h"
#include "code/interface/Control/Objects_Menu_scene.h"
#include "code/interface/Control/Save_Load_Menu_scene.h"
#include "code/interface/Control/Skills_Menu_scene.h"
#include "code/interface/Control/Stats_Menu_scene.h"
#include "code/sprites/chipset.h"
#include "code/readers/map.h"

#include "code/tools/control.h"

#define SCREEN_SIZE_X 320
#define SCREEN_SIZE_Y 240

 bool running = true;
 unsigned  char TheScene=0;
 Mix_Music *musica;
 SDL_Surface * Screen;
 unsigned  char speed=4,timer=0;
 Scene * actual;
 Map_Scene mapas;
 Title_Scene titulo;
 GO_Scene fin;
 Save_Load_Menu_Scene Menu_Save_Load;
 Equip_Menu_Scene Menu_Euip;
 Main_Menu_Scene Menu_Main;
 Objects_Menu_Scene Menu_Objects;
 Stats_Menu_Scene Menu_Stats;
 Skills_Menu_Scene Menu_Skills;
 Item_use_scene Menu_item_use;
 Batle_scene batalla;
 Player_Team team;
 CDeltaTime System(60);
 bool Fullscreen;
 Timer update;
 Timer fps;

    void CalculateFPS()
    {
        char stringBuffer[255];
        static unsigned long nextTicks = 0, fps = 0, frames = 0;
        frames++;
        if ( SDL_GetTicks() > nextTicks )
        {
            fps = frames;
            frames = 0;
            nextTicks = SDL_GetTicks() + 1000;

            sprintf(stringBuffer, "Map test - FPS %lu", fps);
            SDL_WM_SetCaption (stringBuffer, NULL);
        }
    }

int fps_sincronizar()
{
	static unsigned int frames=0;
	int tiempodesignado, tiempotrascurrido,framesideales, retraso;
	frames++;
	if(( update.get_ticks() > 1020 ) ||(frames==60))
	{
		frames=0;
		update.start();
		fps.start();
	}
	else
	{
        tiempodesignado=((1020- update.get_ticks())/(60-frames));
		tiempotrascurrido=fps.get_ticks();
		if(tiempotrascurrido < tiempodesignado)
		{
			SDL_Delay( tiempodesignado- tiempotrascurrido );
			fps.start();
			return 1;
		}
		else
		{
			framesideales= (update.get_ticks())/17;
			retraso=framesideales-frames;
			fps.start();
			if((retraso)>=1)
			{
				frames=frames+retraso;
				return (1+retraso);
			}
		}
	}
	return 1;
}


void CambioScene(Audio * myaudio, Scene** apuntador)
 {
	unsigned  static char LastScene=0;
	if(TheScene!=LastScene)
	{
		(**apuntador).dispose();
		if(TheScene==0)
		{
		    Control::set_delay_default();
			titulo.init( myaudio,& running,& TheScene,& team);
			*apuntador=(& titulo);
			LastScene=0;
		}
		if(TheScene==1)
		{
			mapas.init(myaudio,320,240,& TheScene,& team);
			*apuntador=& mapas;
			LastScene=1;
		}
		if(TheScene==2)
		{
		    Control::set_delay_default();
			batalla.init(myaudio,& running,& TheScene,& team);
			*apuntador=& batalla;
			LastScene=2;
		}
		if(TheScene==3)
		{
		    Control::set_delay_default();
			fin.init(myaudio,& running,& TheScene,& team);
			*apuntador=& fin;
			LastScene=3;
		}
		if(TheScene==4)
		{
		    Control::set_delay_default();
			Menu_Main.init(myaudio,& running,& TheScene,& team);
			*apuntador=& Menu_Main;
			LastScene=4;
		}
		if(TheScene==5)
		{
		    Control::set_delay_default();
			Menu_Objects.init(myaudio,& running,& TheScene,& team);
			*apuntador=& Menu_Objects;
			LastScene=5;
		}
		if(TheScene==6)
		{
		    Control::set_delay_default();
			Menu_Skills.init(myaudio,& running,& TheScene,& team);
			*apuntador=& Menu_Skills;
			LastScene=6;
		}
		if(TheScene==7)
		{
		    Control::set_delay_default();
			Menu_Euip.init(myaudio,& running,& TheScene,& team);
			*apuntador=& Menu_Euip;
			LastScene=7;
		}
		if(TheScene==8)
		{
		    Control::set_delay_default();
			Menu_Stats.init(myaudio,& running,& TheScene,& team);
			*apuntador=& Menu_Stats;
			LastScene=8;
		}
		if(TheScene==9)
		{
		    Control::set_delay_default();
			Menu_Save_Load.init(myaudio,& running,& TheScene,& team);
			*apuntador=& Menu_Save_Load;
			LastScene=9;
		}
		if(TheScene==10)
		{
		    Control::set_delay_default();
			Menu_item_use.init(myaudio,& running,& TheScene,& team);
			*apuntador=& Menu_item_use;
			LastScene=10;
		}

	}
}



int main(int argc, char** argv)
{
    Control::set_keys();
	Music myaudio;
	int repxciclo,i;
	// ===[ INITIALIZATION ]================================================
	// Start SDL
	if (SDL_Init (SDL_INIT_VIDEO| SDL_INIT_AUDIO) < 0)
	{
		exit(1);//mod
	}
	atexit (SDL_Quit);
	myaudio.init();
	const SDL_VideoInfo * videoInfo = SDL_GetVideoInfo();
	unsigned  long flags = 0;

	if (videoInfo->hw_available) flags |= SDL_HWSURFACE;
	else flags |= SDL_SWSURFACE;
	if (videoInfo->blit_hw)flags |= SDL_HWACCEL;
	Screen = SDL_SetVideoMode (SCREEN_SIZE_X, SCREEN_SIZE_Y, 16, flags );
	if (Screen == NULL)
	{
		exit(2);
	}
	SDL_WM_SetCaption ("Player", NULL);

	// ===[ ENTRY POINT ]===================================================

//	SDL_Event event;
	titulo.init(& myaudio,& running,& TheScene,& team);

	actual= & titulo;
	update.start();
	fps.start();

	while (running)
	{
		timer++;
		// Check for events
		/*while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
						running = false;
						break;
				case SDL_KEYDOWN:
						if( event.key.keysym.sym == SDLK_RETURN )
						{
							if(Fullscreen)
							{
								Screen = SDL_SetVideoMode( SCREEN_SIZE_X, SCREEN_SIZE_Y, 16, SDL_SWSURFACE | SDL_RESIZABLE);
								Fullscreen=false;
							}
							else
							{
								Screen = SDL_SetVideoMode( SCREEN_SIZE_X, SCREEN_SIZE_Y, 16, SDL_SWSURFACE | SDL_RESIZABLE | SDL_FULLSCREEN );
								Fullscreen=true;
							}
							break;
						}
				default:
						break;
			}
		}*/
		repxciclo = fps_sincronizar ();
		// SDL_FillRect(Screen, NULL, 0x0);// Clear screen


		for (i = 0; i < repxciclo; i ++)
		{
		    Control::update_keys();
			System.update(); //updates delta
			actual->updatekey( );

        }
		actual->update(Screen);

		CambioScene(& myaudio, & actual);


		SDL_Flip(Screen); // Flip
        CalculateFPS();
	}
	SDL_Quit();
	return false;
}
