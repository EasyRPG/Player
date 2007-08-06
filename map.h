/* map.h, main map routines.
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

#ifndef MAP_H
#define MAP_H

// *****************************************************************************
// =============================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include "SDL.h"
    #include "tools.h"
    #include "chipset.h"
    using namespace std;
// =============================================================================
// *****************************************************************************
        
    // === Map structure =======================================================
    struct stMap
    {
        // --- Fields declaration ---------------------------------------------- 
        short MapWidth, MapHeight;      // These determine the size of the map
        
        short ChipsetID;                // This points to the ID of the Chipset
        stChipset Chipset;              // the map uses, and the structure
                                        // contains the tileset data of it
   
        unsigned char TypeOfLoop;       // These flags determines if the map has to
                                        // loop infinitely.
        
        bool ParallaxBackground;
        string BackgroundName;
            
        bool HorizontalPan;
        bool HorizontalAutoPan;
        short HorizontalPanSpeed;
            
        bool VerticalPan;
        bool VerticalAutoPan;
        short VerticalPanSpeed;
            
        unsigned short * LowerLayer;
        unsigned short * UpperLayer;
            
        int TimesSaved;
        int NumEvents;
        
        // --- Methods declaration ---------------------------------------------
        bool Load(string Filename);
        void GetNextChunk(FILE * Stream);
        void ProcessChunk(FILE * Stream, tChunk * Chunk);
        void ShowInformation();   
        
        void Render(SDL_Surface * Destiny, int Layer, int CameraX, int CameraY);         
    };

#endif
