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
    #include "SDL.h"
    #include "SDL_image.h"
    #include "../tools/tools.h"
    #include "../readers/map.h"

    using namespace std;
// =============================================================================
// *****************************************************************************

    class Chipset
    {
     SDL_Surface * ChipsetSurface;   // Chipset's precalculated surface

     void RenderTile(SDL_Surface * Destiny, int x, int y, unsigned short Tile, int Frame);
public:
     map_data * data;
     void init(SDL_Surface * precalculated,map_data * m_data);
     void Render(SDL_Surface * Destiny, int Layer, int CameraX, int CameraY);

    };
    class Pre_Chipset
    {
    private:
        SDL_Surface * BaseSurface;      // Chipset's base surface!
        void RenderWaterTile(SDL_Surface * Destiny, int x, int y, int Frame, int Border, int Water, int Combination);
        void RenderDepthTile(SDL_Surface * Destiny, int x, int y, int Frame, int Depth, int DepthCombination);
        void RenderTerrainTile(SDL_Surface * Destiny, int x, int y, int Terrain, int Combination);
    public:
        SDL_Surface * ChipsetSurface;   // Chipset's precalculated surface
        bool GenerateFromSurface(SDL_Surface * Surface);
        bool GenerateFromFile(string Filename);
        void dispose();
     };

#endif
