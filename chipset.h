/* chipset.h, types and prototypes for the map tileset management.
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
   
#ifndef CHIPSET_H
#define CHIPSET_H

// *****************************************************************************
// =============================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include <SDL/SDL.h>
    #include "tools.h"
    using namespace std;
// =============================================================================
// *****************************************************************************

    // === Chipset structure ===================================================
    struct stChipset
    {
        // --- Fields declaration ----------------------------------------------
        
        // The chipset structure holds the graphic tileset of a chipset, as well
        // as their properties and the methods for correctly displaying them.
        SDL_Surface * BaseSurface;      // Chipset's base surface!
        SDL_Surface * ChipsetSurface;   // Chipset's precalculated surface
        SDL_Surface * UnsupportedTile;  // Used to denote that tile is unsupported ;o
        
        // --- Methods declaration ---------------------------------------------
        bool GenerateFromSurface(SDL_Surface * Surface);
        bool GenerateFromFile(string Filename, Color KeyColor);
        void Release();
               
        void RenderTile(SDL_Surface * Destiny, int x, int y, unsigned short Tile, int Frame);
        void RenderWaterTile(SDL_Surface * Destiny, int x, int y, int Frame, int Border, int Water, int Combination);
        void RenderDepthTile(SDL_Surface * Destiny, int x, int y, int Frame, int Depth, int DepthCombination);
        void RenderTerrainTile(SDL_Surface * Destiny, int x, int y, int Terrain, int Combination);
    };

#endif
