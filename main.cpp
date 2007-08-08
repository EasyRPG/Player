/* main.cpp, EasyRPG player main file.
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
   
    #include <stdlib.h>
    #include <stdio.h>
    #include "SDL.h"
    #include "tools.h"
    #include "map.h"
    using namespace std;
    
    SDL_Surface * Screen;
    unsigned long nextTicks = 0, fps = 0, frames = 0;
    char stringBuffer[255];
    
    void CalculateFPS()
    {
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
    
    int main(int argCounts, char * argStrings[])
    {       
        // ===[ INITIALIZATION ]================================================
        // Start SDL
        if (SDL_Init (SDL_INIT_VIDEO) < 0) exit(1);
        atexit (SDL_Quit);
    
        const SDL_VideoInfo * videoInfo = SDL_GetVideoInfo();
        unsigned long flags = 0;
        
        if (videoInfo->hw_available) flags |= SDL_HWSURFACE;
        else                         flags |= SDL_SWSURFACE;
        if (videoInfo->blit_hw)      flags |= SDL_HWACCEL;
        
        // Start screen (set to 320x240)
        Screen = SDL_SetVideoMode (320, 240, 16, flags );
        if (Screen == NULL) exit(2);
        SDL_WM_SetCaption ("Map test", NULL);
        
        // ===[ LOADING MAP DATA ]==============================================
        stMap Map;
        if (argCounts!=2) Map.Load("Map0001.lmu");
        else              Map.Load(argStrings[1]);       
        Map.ShowInformation(); 
        Map.Chipset.GenerateFromFile("Basis.png");
	        
        // ===[ ENTRY POINT ]===================================================
        // Main loop
        bool running = true;        
        SDL_Event event;
        unsigned char * keyData;
        int cX = 0, cY = 0;
        
        while (running)
        {
            // Check for events
            while (SDL_PollEvent (&event));
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        running = false;
                        break;
                    default:
                        break;
                }
            }

            keyData = SDL_GetKeyState(NULL);
            if ( keyData[SDLK_ESCAPE] ) running = false;
            
            if ( keyData[SDLK_LEFT]  ) cX--;
            if ( keyData[SDLK_RIGHT] ) cX++;
            if ( keyData[SDLK_UP]    ) cY--;
            if ( keyData[SDLK_DOWN]  ) cY++;
            
            // Clear screen
            SDL_FillRect(Screen, NULL, 0x0);
            
                Map.Render(Screen, 0, cX, cY);
                Map.Render(Screen, 1, cX, cY);
                
            // Flip
            SDL_Flip(Screen);
            
            // Get FPS
            CalculateFPS();
            
            // Delay to 60 fps
//            SDL_Delay(1000/60);
        }
        return true;
    }
