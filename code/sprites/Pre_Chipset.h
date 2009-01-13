#ifndef PRECHIPSET_H
#define PRECHIPSET_H

// *****************************************************************************
// =============================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include "SDL.h"
    #include "SDL_image.h"
    #include "../tools/tools.h"
    #include "../readers/map.h"
    #include "../readers/ldbstr.h"

    using namespace std;
// =============================================================================
// *****************************************************************************

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
