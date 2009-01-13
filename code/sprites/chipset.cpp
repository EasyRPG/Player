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
    #include "../tools/tools.h"
    #include "chipset.h"
    using namespace std;

// =============================================================================
// *****************************************************************************

    void Chipset::init(SDL_Surface * precalculated,map_data * m_data,stcChipSet * title_colision)
    {
        ChipsetSurface=precalculated;
        data=m_data;
        upper_colision_matrix = new unsigned short[(data->MapHeight)*(data->MapWidth)];
        lower_colision_matrix = new unsigned short[(data->MapHeight)*(data->MapWidth)];
        //title_colision->show();
        colision_base(0,title_colision);
        colision_base(1,title_colision);

    }

    void Chipset::Render(SDL_Surface * Destiny, int Layer, int CameraX, int CameraY)
    {
        // Declarate the variables we're going to use...
        int x, y, xStart, xEnd, yStart, yEnd;
	    //int Tile;
        unsigned short * TilePointer;
        unsigned long  StepPerY;
        int Frame = SDL_GetTicks()>>7;
        //SDL_Rect destinyRect;

        // Calculate boundaries
        xStart = (CameraX>>4) - 1;
        yStart = (CameraY>>4) - 1;
        xEnd   = ((CameraX + Destiny->w)>>4) + 1;
        yEnd   = ((CameraY + Destiny->h)>>4) + 1;

        // Clipping
        if ( xStart < 0        ) xStart = 0;
        if ( yStart < 0        ) yStart = 0;
        if ( xEnd   > (data->MapWidth) ) xEnd   = data->MapWidth;
        if ( yEnd   > (data->MapHeight)) yEnd   = data->MapHeight;

        switch(Layer)
        {
            case 0:
                // Calculate initial pointer and step
                TilePointer = &data->LowerLayer[xStart + yStart * (data->MapWidth)];
                StepPerY    = data->MapWidth - (xEnd-xStart);

                // Run through the whole map
                for ( y=yStart; y<yEnd; y++, TilePointer+=StepPerY )
                    for ( x = xStart; x < xEnd; x++, TilePointer++ )
                        RenderTile(Destiny, (x<<4)-CameraX, (y<<4)-CameraY, *TilePointer, Frame);
                break;
            case 1:
                // Calculate initial pointer and step
                TilePointer = &data->UpperLayer[xStart + yStart * (data->MapWidth)];
                StepPerY    = data->MapWidth - (xEnd-xStart);

                // Run through the whole map
                for ( y=yStart; y<yEnd; y++, TilePointer+=StepPerY )
                    for ( x = xStart; x < xEnd; x++, TilePointer++ )
                        RenderTile(Destiny, (x<<4)-CameraX, (y<<4)-CameraY, *TilePointer, Frame);
                break;
        }
    }


   void Chipset::RenderTile(SDL_Surface * Destiny, int x, int y, unsigned short Tile, int Frame)
    {

//        static bool was_deph=false;
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
            int WaterTile =  (Tile%50);
            int WaterType = ((Tile/50)/20);
            int Shadow =(Tile/50);

            Tile = WaterType*141+WaterTile+(Frame*47);
            DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);
            if (WaterType==2)// if is a deph water title redraw the shadow.
            {       Tile = 3*141+Shadow-(20*WaterType)+(Frame*16);
                    DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);

            }else
            {

                    Tile = 3*141+Shadow-(20*WaterType)+(Frame*16)+48;
                    DrawSurface(Destiny, x, y, ChipsetSurface, ((Tile&0x1F)<<4), ((Tile>>5)<<4), 16, 16);
           }

        }
    }

    void Chipset::colision_base(int Layer,stcChipSet * title_colision)
    {
        int x, y, xStart, xEnd, yStart, yEnd;
	    unsigned short * TilePointer;
        xStart = 0;
        yStart = 0;
        xEnd   = data->MapWidth;
        yEnd   = data->MapHeight;

        switch(Layer)
        {
            case 0:
                // Calculate initial pointer and step
                TilePointer = &data->LowerLayer[0];
                // Run through the whole map
                for ( y=yStart; y<yEnd; y++ )
                    for ( x = xStart; x < xEnd; x++, TilePointer++ )
                        lower_colision_matrix[(x + (y*(data->MapWidth)))]=title_colision->vc_ch_Lower_tile_passable[colision_cal(*TilePointer)];
                break;
            case 1:
                // Calculate initial pointer and step
                TilePointer = &data->UpperLayer[0];
                // Run through the whole map
                for ( y=yStart; y<yEnd; y++)
                    for ( x = xStart; x < xEnd; x++, TilePointer++ )
                        upper_colision_matrix[(x + (y*(data->MapWidth)))]=title_colision->vc_ch_Upper_tile_passable[colision_cal(*TilePointer)];
                break;
        }
    }



   unsigned short Chipset::colision_cal(unsigned short Tile)
    {
        if (Tile >= 0x2710)         // Upper layer tiles
        {
            Tile -= 0x2710;
        } else if (Tile >= 0x1388)  // Lower layer tiles
        {
            Tile -= 0x1388;
            Tile += 18;
        } else if (Tile >= 0x0FA0)  // Terrain tiles
        {
            Tile -= 0x0FA0;
            Tile /= 50;
            Tile += 6;
        } else if (Tile >= 0x0BB8)  // Animated tiles
        {
            Tile =3+((Tile-0x0BB8)/50);
        } else {                    // Water tiles
            Tile = ((Tile/50)/20);
        }
    return(Tile);
    }

bool Chipset::CollisionAt(int x, int y, unsigned short dir)
{
    int plain_id=x + (y*(data->MapWidth));
    //solo los primeros 4 bits
    int id_upper=(upper_colision_matrix[plain_id]& (0x0F));
    int id_lower=(lower_colision_matrix[plain_id]& (0x0F));

        switch(dir)
        {

//arriba 4 bit 8
//derecha 3 bit 4
//izquierda 2 bit 2
//abajo 1 bit 1

            case 0://ACTOR_DIRECTION_UP
                if(y==0)
                    return(false);
                if((id_lower>>3)&&(id_upper>>3))
                {
                    plain_id=x + ((y-1)*(data->MapWidth));
                    id_upper=(upper_colision_matrix[plain_id]& (0x0F));
                    id_lower=(lower_colision_matrix[plain_id]& (0x0F));

                    if((id_lower& (0x01))&&(id_upper& (0x01)))
                        return(true);
                }
                return(false);

            break;

            case 1: //ACTOR_DIRECTION_DOWN
                if((y+1)==data->MapHeight)
                    return(false);

                if((id_lower& (0x01))&&(id_upper& (0x01)))
                {
                    plain_id=x + ((y+1)*(data->MapWidth));
                    id_upper=(upper_colision_matrix[plain_id]& (0x0F));
                    id_lower=(lower_colision_matrix[plain_id]& (0x0F));

                    if((id_lower>>3)&&(id_upper>>3))
                        return(true);
                }
                return(false);
            break;

            case 2: //ACTOR_DIRECTION_LEFT
                   if(x==0)
                        return(false);

                if(((id_lower>>1)& (0x01))&&((id_upper>>1)& (0x01)))
                {
                    plain_id=x + (y*(data->MapWidth))-1;
                    id_upper=(upper_colision_matrix[plain_id]& (0x0F));
                    id_lower=(lower_colision_matrix[plain_id]& (0x0F));

                    if(((id_lower>>2)& (0x01))&&((id_upper>>2)& (0x01)))
                        return(true);
                }
                return(false);
            break;

            case 3://ACTOR_DIRECTION_RIGHT
                   if((x+1)==data->MapWidth)
                        return(false);

                if(((id_lower>>2)& (0x01))&&((id_upper>>2)& (0x01)))
                {
                    plain_id=x + (y*(data->MapWidth))+1;
                    id_upper=(upper_colision_matrix[plain_id]& (0x0F));
                    id_lower=(lower_colision_matrix[plain_id]& (0x0F));

                    if(((id_lower>>1)& (0x01))&&((id_upper>>1)& (0x01)))
                        return(true);
                }
                return(false);
            break;

        }
 return(false);
}
