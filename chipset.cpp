/* chipset.cpp, routines for the map tileset management.
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

    // === Chipset structure ===================================================
    bool stChipset::GenerateFromSurface(SDL_Surface * Surface)
    {
        // Set base surface, used for generating the tileset
        BaseSurface     = Surface;
        ChipsetSurface  = CreateSurface(32*16, 45*16);

        int CurrentTile = 0;

       	// Generate water A
        for (int j=0; j<3; j++)
            for (int i=0; i<47; i++, CurrentTile++)
                RenderWaterTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, 0, 0, i);

        // Generate water B
        for (int j=0; j<3; j++)
            for (int i=0; i<47; i++, CurrentTile++)
                RenderWaterTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, 1, 0, i);

        // Generate water C
        for (int j=0; j<3; j++)
            for (int i=0; i<47; i++, CurrentTile++)
                RenderWaterTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, 0, 3, i);

        // Generate water depth tiles
        for (int i=0; i<48; i++, CurrentTile++)
            RenderDepthTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, i/16, 1, i%16);

        for (int i=0; i<48; i++, CurrentTile++)
            RenderDepthTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, i/16, 3, i%16);

        // Generate animated tiles
        for (int j=0; j<3; j++)
            for (int i=0; i<4; i++, CurrentTile++)
                DrawSurface(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, BaseSurface, 48+j*16, 64+i*16, 16, 16);

        // Generate terrain tiles
        for (int j=0; j<12; j++)
            for (int i=0; i<50; i++, CurrentTile++)
                RenderTerrainTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, i);

        // Generate common tiles
        for (int i=0; i<288; i++, CurrentTile++)
            DrawSurface(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, BaseSurface, 192+((i%6)*16)+(i/96)*96, ((i/6)%16)*16, 16, 16);

        // Done
        return true;
    }

    bool stChipset::GenerateFromFile(string Filename)
    {
        SDL_Surface * FileSurface = LoadSurface(Filename);
        return GenerateFromSurface(FileSurface);
    }


    // =========================================================================
    void stChipset::RenderTile(SDL_Surface * Destiny, int x, int y, unsigned short Tile, int Frame)
    {
        if (Tile >= 0x2710)         // Upper layer tiles
        {
            Tile -= 0x2710;
            Tile += 0x04FB;
            DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);
        } else if (Tile >= 0x1388)  // Lower layer tiles
        {
            Tile -= 0x1388;
            Tile += 0x046B;
            DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);
        } else if (Tile >= 0x0FA0)  // Terrain tiles
        {
            Tile -= 0x0FA0;
            Tile += 0x0213;
            DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);
        } else if (Tile >= 0x0BB8)  // Animated tiles
        {
            Frame %= 4;
            Tile = 0x0207 + (((Tile-0x0BB8)/50)<<2) + Frame;
            DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);
        } else {                    // Water tiles
            Frame %= 3;
            int WaterTile =  Tile%50;
            int WaterType = ((Tile/50)/20);
            Tile = WaterType*141+WaterTile+(Frame*47);

            DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);
        }
    }

    void stChipset::RenderWaterTile(SDL_Surface * Destiny, int x, int y, int Frame, int Border, int Water, int Combination)
    {
        int SFrame       = Frame*16, SBorder = Border*48;
        //int SFrameBorder = SFrame+SBorder;
        Combination &= 0x3F;

        // Since this function isn't meant to be used in realtime, we can allow
        // use nasty code here. First off, draw the water tile, for the background.
        DrawSurface(Destiny, x, y, BaseSurface, SFrame, 64+(Water*16), 16, 16);

        // Now, get the combination from the tile and draw it using this stupidly
        // hard coded routine. I've found out that this was easier than just find
        // out a damn algorithm.
        if (Combination & 0x20)
        {
            // This is where it gets nasty :S
            if (Combination > 0x29)
            {
                // Multiple edge possibilities
                switch(Combination)
                {
                    case 0x2A:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 16, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 24, 16, 8);
                        break;
                    case 0x2B:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 8, 16);
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 32, 8, 16);
                        break;
                    case 0x2C:
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 8, 16, 8);
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 16, 8);
                        break;
                    case 0x2D:
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 0, 8, 16);
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 8, 16);
                        break;
                    case 0x2E:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 16, 16);
                        break;
                }
            } else {
                // Wall + inner edges
                switch((Combination>>1)&0x07)
                {
                    case 0x00:
                        if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 16, 16);
                        else                  DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 16, 16);
                        break;
                    case 0x01:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 8, 8);           // Corner
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 24, 8, 8);        // Left/Right frame
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 32, 8, 8);      // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
                        break;
                    case 0x02:
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 0, 8, 8);       // Corner
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 24, 8, 8);    // Left/Right frame
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 8, 8);          // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 48+8, 8, 8);
                        break;
                    case 0x03:
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 8, 8, 8);     // Corner
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 16, 8, 8);      // Left/Right frame
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 40, 8, 8);        // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 48, 8, 8);
                        break;
                    case 0x04:
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 8, 8, 8);         // Corner
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 8, 8);          // Left/Right frame
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 40, 8, 8);    // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 48, 8, 8);
                        break;
                }
            }
        } else if (Combination & 0x10)
        {
            // Wall + inner edge cases. They're also easier to find out the
            // values here too
            switch((Combination>>2)&0x03)
            {
                case 0x00:
                    // Render left wall
                    DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 8, 16);

                    // Render top right corner and bottom right corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y,   BaseSurface, SFrame+SBorder+8, 48, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
                    break;
                case 0x01:
                    // Render top wall
                    DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 16, 8);

                    // Render bottom right corner and bottom left corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y+8,   BaseSurface, SFrame+SBorder, 48+8, 8, 8);
                    break;
                case 0x02:
                    // Right wall
                    DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 16, 8, 16);

                    // Render bottom left corner and top left corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 48+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y,   BaseSurface, SFrame+SBorder, 48, 8, 8);
                    break;
                case 0x03:
                    // Bottom wall
                    DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 32+8, 16, 8);

                    // Render top left corner and top right corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y,   BaseSurface, SFrame+SBorder, 48, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 48, 8, 8);
                    break;
            }
        } else {
            // Single inner edge cases. They're easier to find out the values
            // this way.
            if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 48, 8, 8);
            if (Combination&0x02) DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 48, 8, 8);
            if (Combination&0x04) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
            if (Combination&0x08) DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 48+8, 8, 8);
        }
    }

    void stChipset::RenderTerrainTile(SDL_Surface * Destiny, int x, int y, int Terrain, int Combination)
    {
        Terrain += 4;
        int XTerrain = ((Terrain%2)*48)+(Terrain/8)*96, YTerrain = ((Terrain/2)%4)*64;


        // Since this function isn't meant to be used in realtime, we can allow
        // use nasty code here. First off, draw the water tile, for the background.
        DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+32, 16, 16);

        // Now, get the combination from the tile and draw it using this stupidly
        // hard coded routine. I've found out that this was easier than just find
        // out a damn algorithm.
        if (Combination & 0x20)
        {
            // This is where it gets nasty :S
            if (Combination > 0x29)
            {
                // Multiple edge possibilities
                switch(Combination)
                {
                    case 0x2A:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+16, 8, 16);
                        DrawSurface(Destiny, x+8, y, BaseSurface,   XTerrain+40, YTerrain+16, 8, 16);
                        break;
                    case 0x2B:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+16, 16, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface,   XTerrain,    YTerrain+56, 16, 8);
                        break;
                    case 0x2C:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+48, 8, 16);
                        DrawSurface(Destiny, x+8, y, BaseSurface,   XTerrain+40, YTerrain+48, 8, 16);
                        break;
                    case 0x2D:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain+32, YTerrain+16, 16, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface,   XTerrain+32, YTerrain+56, 16, 8);
                        break;
                    case 0x2E:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+16, 8, 8);
                        DrawSurface(Destiny, x+8, y, BaseSurface,   XTerrain+40, YTerrain+16, 8, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface,   XTerrain,    YTerrain+56, 8, 8);
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+56, 8, 8);
                        break;
                    case 0x31:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain, 16, 16);
                        break;
                }
            } else {
                // Wall + inner edges
                switch((Combination>>1)&0x07)
                {
                    case 0x00:
                        if (Combination&0x01)
                        {
                            DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+16, 16, 8);
                            DrawSurface(Destiny, x, y+8, BaseSurface, XTerrain+16, YTerrain+56, 16, 8);
                        } else {
                            DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+32, 8, 16);
                            DrawSurface(Destiny, x+8, y, BaseSurface, XTerrain+40, YTerrain+32, 8, 16);
                        }
                        break;
                    case 0x01:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+16, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
                        break;
                    case 0x02:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain+16, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x, y+8, BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
                        break;
                    case 0x03:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain+48, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain, 8, 8);
                        break;
                    case 0x04:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+48, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x+8, y, BaseSurface, XTerrain+40, YTerrain, 8, 8);
                        break;
                }
            }
        } else if (Combination & 0x10)
        {
            // Wall + inner edge cases. They're also easier to find out the
            // values here too
            switch((Combination>>2)&0x03)
            {
                case 0x00:
                    // Render left wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+32, 16, 16);

                    // Render top right corner and bottom right corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y,   BaseSurface, XTerrain+40, YTerrain, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
                    break;
                case 0x01:
                    // Render top wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+16, 16, 16);

                    // Render bottom right corner and bottom left corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y+8,   BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
                    break;
                case 0x02:
                    // Right wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain+32, 16, 16);

                    // Render bottom left corner and top left corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y+8,   BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y,     BaseSurface, XTerrain+32, YTerrain, 8, 8);
                    break;
                case 0x03:
                    // Bottom wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+48, 16, 16);

                    // Render top left corner and top right corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y,     BaseSurface, XTerrain+32, YTerrain, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y,   BaseSurface, XTerrain+40, YTerrain, 8, 8);
                    break;
            }
        } else {
            // Single inner edge cases. They're easier to find out the values
            // this way.
            if (Combination&0x01) DrawSurface(Destiny, x, y,     BaseSurface, XTerrain+32, YTerrain, 8, 8);
            if (Combination&0x02) DrawSurface(Destiny, x+8, y,   BaseSurface, XTerrain+40, YTerrain, 8, 8);
            if (Combination&0x04) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
            if (Combination&0x08) DrawSurface(Destiny, x, y+8,   BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
        }
    }

    void stChipset::RenderDepthTile(SDL_Surface * Destiny, int x, int y, int Frame, int Depth, int DepthCombination)
    {
        int SFrame = Frame*16;

        // Now render the depth part
        if (DepthCombination&0x01) DrawSurface(Destiny, x,   y,   BaseSurface, SFrame,   64+(Depth*16),   8, 8);
        if (DepthCombination&0x02) DrawSurface(Destiny, x+8, y,   BaseSurface, SFrame+8, 64+(Depth*16),   8, 8);
        if (DepthCombination&0x04) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+8, 64+(Depth*16)+8, 8, 8);
        if (DepthCombination&0x08) DrawSurface(Destiny, x,   y+8, BaseSurface, SFrame,   64+(Depth*16)+8, 8, 8);
    }

    // =========================================================================

