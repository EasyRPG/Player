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
 //
#include <vector>
#include <string>
//#include "tools/math-sll.h"
#include "tools/audio.h"
#include "tools/CDeltaTime.h"
#include "tools/control.h"
#include "tools/font.h"
#include "tools/key.h"
#include "tools/tools.h"
#include "tools/Timer.h"
#include "sprites/chipset.h"
#include "sprites/sprite.h"
#include "control/Batle_scene.h"
#include "control/GameOver_scene.h"
#include "control/Map_scene.h"
#include "control/scene.h"
#include "control/Title_scene.h"
#include "interface/Control/Equip_Menu_scene.h"
#include "interface/Control/Item_use_scene.h"
#include "interface/Control/Main_Menu_scene.h"
#include "interface/Control/Objects_Menu_scene.h"
#include "interface/Control/Save_Load_Menu_scene.h"
#include "interface/Control/Skills_Menu_scene.h"
#include "interface/Control/Stats_Menu_scene.h"
#include "readers/iniparser.h"
#include "readers/map.h"
#include "attributes/CActor.h"
#include "attributes/Enemy.h"
#include "attributes/item.h"
#include "attributes/Player.h"
#include "attributes/skill.h"


#ifdef PSP
#define SCREEN_SIZE_X 480
#define SCREEN_SIZE_Y 272
#else
#define SCREEN_SIZE_X 320
#define SCREEN_SIZE_Y 240
#endif

//#define SCREEN_SIZE_2X
Scene *actual;

SDL_Surface *Screen;
SDL_Surface *ScreenZ;
unsigned char speed = 4, timer = 0;
Map_Scene * mapas;
Title_Scene * titulo;
GO_Scene * fin;
Save_Load_Menu_Scene * Menu_Save_Load;
Equip_Menu_Scene * Menu_Euip;
Main_Menu_Scene * Menu_Main;
Objects_Menu_Scene * Menu_Objects;
Stats_Menu_Scene * Menu_Stats;
Skills_Menu_Scene * Menu_Skills;
Item_use_scene * Menu_item_use;
Batle_scene * batalla;
General_data team;
CDeltaTime System(60);
bool Fullscreen;
Timer update;
Timer fps;

void CalculateFPS()
{
    static unsigned long nextTicks = 0, fps = 0, frames = 0;
    frames++;
    if ( SDL_GetTicks() > nextTicks )
    {
        fps = frames;
        frames = 0;
        nextTicks = SDL_GetTicks() + 1000;

       // printf("Map test - FPS %lu /n \n", fps);
        //SDL_WM_SetCaption (stringBuffer, NULL);
    }
}

int fps_sincronizar()
{
    static unsigned int frames = 0;
    int tiempodesignado, tiempotrascurrido, framesideales, retraso;
    frames++;
    if ((update.get_ticks() > 1000 ) || (frames == 60))
    {
        frames = 0;
        update.start();
        fps.start();
    }
    else
    {
        tiempodesignado = (1000 - update.get_ticks()) / (60 - frames);
        tiempotrascurrido = fps.get_ticks();
        if (tiempotrascurrido < tiempodesignado)
        {
            SDL_Delay(tiempodesignado - tiempotrascurrido);
            fps.start();
            return 1;
        }
        else
        {
            framesideales = update.get_ticks() / 17;
            retraso = framesideales - frames;
            fps.start();
            if (retraso >= 1)
            {
                frames = frames + retraso;
                return (1 + retraso);
            }
        }
    }
    return 1;
}

void CambioScene(Scene **apuntador)
{
    unsigned static char LastScene = 0;
    if (team.TheScene != LastScene)
    {
        (*apuntador)->dispose();
      //  delete (*apuntador);


        if (LastScene == 0)
        {
         Title_Scene * ptrdst0;
         ptrdst0=(Title_Scene*) (*apuntador);
         delete(ptrdst0);
        }
        if (LastScene == 1)
        {
         Map_Scene * ptrdst1;
         ptrdst1=(Map_Scene*) (*apuntador);
         delete(ptrdst1);
        }
         if (LastScene == 2)
        {
         Batle_scene * ptrdst2;
         ptrdst2=(Batle_scene*) (*apuntador);
         delete(ptrdst2);
        }
         if (LastScene == 3)
        {
         GO_Scene * ptrdst3;
         ptrdst3=(GO_Scene*) (*apuntador);
         delete(ptrdst3);
        }
        if (LastScene == 4)
        {
         Main_Menu_Scene * ptrdst4;
         ptrdst4=(Main_Menu_Scene*) (*apuntador);
         delete(ptrdst4);
        }

        if (LastScene == 5)
        {
         Objects_Menu_Scene * ptrdst5;
         ptrdst5=(Objects_Menu_Scene*) (*apuntador);
         delete(ptrdst5);
        }

        if (LastScene == 6)
        {
         Skills_Menu_Scene * ptrdst6;
         ptrdst6=(Skills_Menu_Scene*) (*apuntador);
         delete(ptrdst6);
        }

        if (LastScene == 7)
        {
         Equip_Menu_Scene * ptrdst7;
         ptrdst7=(Equip_Menu_Scene*) (*apuntador);
         delete(ptrdst7);
        }

        if (LastScene == 8)
        {
         Stats_Menu_Scene * ptrdst8;
         ptrdst8=(Stats_Menu_Scene*) (*apuntador);
         delete(ptrdst8);
        }

        if (LastScene == 9)
        {
         Save_Load_Menu_Scene * ptrdst9;
         ptrdst9=(Save_Load_Menu_Scene*) (*apuntador);
         delete(ptrdst9);
        }

        if (LastScene == 10)
        {
         Item_use_scene * ptrdst10;
         ptrdst10=(Item_use_scene*) (*apuntador);
         delete(ptrdst10);
        }

        if ( team.TheScene == 0)
        {
            Control::set_delay_default();
            titulo = new Title_Scene();
            titulo->init( &team);
            *apuntador = titulo;
            LastScene = 0;
        }
        if (team.TheScene == 1)
        {
            mapas = new Map_Scene();
            mapas->init(320, 240, &team);
            *apuntador = mapas;
            LastScene=1;
        }
        if (team.TheScene==2)
        {
            Control::set_delay_default();
            batalla=new Batle_scene();
            batalla->init( &team);
            *apuntador = batalla;
            LastScene = 2;
        }
        if (team.TheScene == 3)
        {
            Control::set_delay_default();
            fin= new GO_Scene();
            fin->init(&team);
            *apuntador = fin;
            LastScene = 3;
        }
        if (team.TheScene == 4)
        {
            Control::set_delay_default();
            Menu_Main=new Main_Menu_Scene();
            Menu_Main->init(&team);
            *apuntador = Menu_Main;
            LastScene = 4;
        }
        if (team.TheScene == 5)
        {
            Control::set_delay_default();
            Menu_Objects= new Objects_Menu_Scene();
            Menu_Objects->init(&team);
            *apuntador = Menu_Objects;
            LastScene = 5;
        }
        if (team.TheScene == 6)
        {
            Control::set_delay_default();
            Menu_Skills=new Skills_Menu_Scene();
            Menu_Skills->init(&team);
            *apuntador = Menu_Skills;
            LastScene = 6;
        }
        if (team.TheScene == 7)
        {
            Control::set_delay_default();
            Menu_Euip = new Equip_Menu_Scene();
            Menu_Euip->init(&team);
            *apuntador= Menu_Euip;
            LastScene = 7;
        }
        if (team.TheScene == 8)
        {
            Control::set_delay_default();
            Menu_Stats= new Stats_Menu_Scene();
            Menu_Stats->init(&team);
            *apuntador = Menu_Stats;
            LastScene = 8;
        }
        if (team.TheScene == 9)
        {
            Control::set_delay_default();
            Menu_Save_Load=new Save_Load_Menu_Scene();
            Menu_Save_Load->init(&team);
            *apuntador = Menu_Save_Load;
            LastScene = 9;
        }
        if (team.TheScene == 10)
        {
            Control::set_delay_default();
            Menu_item_use=new Item_use_scene();
            Menu_item_use->init(&team);
            *apuntador = Menu_item_use;
            LastScene = 10;
        }
    }
}


void blitVirtualSurface(SDL_Surface * source, SDL_Surface * destination)
{
  float scaleFactor;
  SDL_Rect offset;

    // pillarbox; compute horizontal offset
    scaleFactor = 2;//float(destination->h) / source->h;
    offset.x = 0;//int((float(destination->w) - (scaleFactor * source->w)) / 2 + 0.5f);
    offset.y = 0;
    SDL_Surface * temp;

    #ifdef PSP
    temp = source;
    offset.x = 80;//int((float(destination->w) - (scaleFactor * source->w)) / 2 + 0.5f);
    offset.y = 16;
    #else
    temp = zoomSurface(source, scaleFactor, scaleFactor, 1);
    #endif

    SDL_FillRect(destination, NULL, 0x0);// Clear screen  inutil
    SDL_BlitSurface(temp, NULL, destination, &offset);
    #ifndef PSP
        SDL_FreeSurface(temp);
    #endif
    team.screen_got_refresh=false;
}


#ifdef PSP
extern "C"
#endif
int main(int argc, char *argv[])
{
    #ifdef PSP
    freopen("stdout", "w", stdout);
    freopen("stderr", "w", stderr);
    #endif

    if (argc >= 2) //Dummy for now, avoiding "unused vars" warnings
    {
        for (int i = 0; i < argc; i++)
        {
            printf("Arg %d: %s\n", i, argv[i]);
        }
    }

    Music myaudio;
    int repxciclo,i;
    dictionary *ini;
    // ===[ INITIALIZATION ]================================================
    // Start SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
    {
        exit(1);//mod
    }

    atexit (SDL_Quit);
    SDL_ShowCursor(SDL_DISABLE);
    myaudio.init();

    Font::init_TTF();

//    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    unsigned  long flags = 0;

    flags |= SDL_SWSURFACE;


    #ifdef SCREEN_SIZE_2X
        #ifdef PSP
            flags |= SDL_FULLSCREEN;
            ScreenZ = SDL_SetVideoMode(SCREEN_SIZE_X, SCREEN_SIZE_Y,16, flags);
            Screen=  CreateSurface(320,240);
        #else
            flags |= SDL_FULLSCREEN;
            ScreenZ = SDL_SetVideoMode(SCREEN_SIZE_X*2, SCREEN_SIZE_Y*2,16, flags);
            Screen=  CreateSurface(SCREEN_SIZE_X,SCREEN_SIZE_Y);
        #endif
    #else
        Screen = SDL_SetVideoMode(SCREEN_SIZE_X, SCREEN_SIZE_Y,16, flags);
    #endif

    if (Screen == NULL)
    {
        exit(2);
    }

    ini = iniparser_new("RPG_RT.ini");
    SDL_WM_SetCaption (iniparser_getstring(ini, "RPG_RT:GameTitle", "Untitled"), NULL);
    iniparser_free(ini);

    Control::set_keys();

    // ===[ ENTRY POINT ]===================================================

    titulo=new Title_Scene();
    titulo->init(&team);

    actual = titulo;
    update.start();
    fps.start();

    while (team.running)
    {
        timer++;
        // Check for events
        repxciclo = fps_sincronizar();
        for (i = 0; i < repxciclo; i++)
        {
            Control::update_keys();
            System.update(); //updates delta
            actual->updatekey();
        }
        actual->update(Screen);

    #ifdef SCREEN_SIZE_2X
      if(team.screen_got_refresh)
        blitVirtualSurface(Screen, ScreenZ);
        SDL_Flip(ScreenZ); // Flip
    #else
        SDL_Flip(Screen); // Flip
    #endif

    CambioScene(&actual);
    CalculateFPS();
    //SDL_Delay(100);
    }

    Control::cleanup();
    Font::Quit();
    SDL_Quit();

    #ifdef PSP
        fclose(stdout);
        fclose(stderr);
    #endif

    return 0;
}
