/* map.cpp, main map routines.
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

    // =========================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include "SDL.h"
    #include "../tools/tools.h"
    #include "map.h"
    // =========================================================================

    // --- Create list of chunks opcodes ---------------------------------------
    enum eMapChunks
    {
        CHUNK_MAP_END_OF_BLOCK      = 0x00,
        CHUNK_MAP_CHIPSET           = 0x01,

        CHUNK_MAP_WIDTH             = 0x02,
        CHUNK_MAP_HEIGHT            = 0x03,

        CHUNK_MAP_TYPE_OF_LOOP      = 0x0B,
        CHUNK_MAP_PARALLAX_BACK     = 0x1F,
        CHUNK_MAP_BACKGROUND        = 0x20,

        CHUNK_MAP_HORIZONTAL_PAN    = 0x21,
        CHUNK_MAP_VERTICAL_PAN      = 0x22,

        CHUNK_MAP_HORIZONTAL_PAN_A  = 0x23,
        CHUNK_MAP_HORIZONTAL_PAN_SP = 0x24,

        CHUNK_MAP_VERTICAL_PAN_A    = 0x25,
        CHUNK_MAP_VERTICAL_PAN_SP   = 0x26,

        CHUNK_MAP_LOWER_LAYER       = 0x47,
        CHUNK_MAP_UPPER_LAYER       = 0x48,
        CHUNK_MAP_EVENTS_LAYER      = 0x51,

        CHUNK_MAP_TIMES_SAVED       = 0x5B
    };

    enum eMapEventChunks
    {
        CHUNK_EVENT_NAME            = 0x01,
        CHUNK_EVENT_X               = 0x02,
        CHUNK_EVENT_Y               = 0x03,
        CHUNK_EVENT_PAGES           = 0x05
    };

    // --- Map class -----------------------------------------------------------
    bool stMap::Load(string Filename)
    {
        // Open map file to read
        FILE * Stream;
        Stream = fopen(Filename.c_str(), "rb");

        // Get header and check if it's a valid file
        string Header = ReadString(Stream);
        if (Header != "LcfMapUnit")
        {
            printf("Reading error: File is not a valid RPG2000 map\n");
            fclose(Stream);
            return false;
        }

        // Set default data of the map
        ChipsetID = 1;
        MapWidth = 20;
        MapHeight = 15;
        TypeOfLoop = 0;
        ParallaxBackground = false;
        string BackgroundName = "None";
        HorizontalPan = false;
        HorizontalAutoPan = false;
        HorizontalPanSpeed = 0;
        VerticalPan = false;
        VerticalAutoPan = false;
        VerticalPanSpeed = 0;
        LowerLayer = NULL;
        UpperLayer = NULL;
        NumEvents = 0;

        // Get data from map
        GetNextChunk(Stream);

        // Close stream
        fclose(Stream);

        // Done
        return true;
    }

    void stMap::GetNextChunk(FILE * Stream)
    {
        tChunk ChunkInfo;
        unsigned char Void;

        // Loop while we haven't reached the end of the file
        while(!feof(Stream))
        {
            ChunkInfo.ID     = ReadCompressedInteger(Stream);
            ChunkInfo.Length = ReadCompressedInteger(Stream);

            switch(ChunkInfo.ID)
            {
                case CHUNK_MAP_CHIPSET:
                    ChipsetID        = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_WIDTH:
                    MapWidth         = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_HEIGHT:
                    MapHeight        = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_TYPE_OF_LOOP:
                    TypeOfLoop       = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_PARALLAX_BACK:
                    ParallaxBackground = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_BACKGROUND:
                    BackgroundName   = ReadString(Stream, ChunkInfo.Length);
                    break;

                case CHUNK_MAP_HORIZONTAL_PAN:
                    HorizontalPan    = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_VERTICAL_PAN:
                    VerticalPan      = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_HORIZONTAL_PAN_A:
                    HorizontalAutoPan = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_HORIZONTAL_PAN_SP:
                    HorizontalPanSpeed = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_VERTICAL_PAN_A:
                    VerticalAutoPan  = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_VERTICAL_PAN_SP:
                    VerticalPanSpeed = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_LOWER_LAYER:
                    // Allocate lower map layer
                    LowerLayer = new unsigned short[ChunkInfo.Length>>1];
                    fread(LowerLayer, sizeof(char), ChunkInfo.Length, Stream);
                    break;

                case CHUNK_MAP_UPPER_LAYER:
                    // Allocate upper map layer
                    UpperLayer = new unsigned short[ChunkInfo.Length>>1];
                    fread(UpperLayer, sizeof(char), ChunkInfo.Length, Stream);
                    break;

//                case CHUNK_MAP_EVENTS_LAYER:
//                    break;

                case CHUNK_MAP_TIMES_SAVED:
                    TimesSaved = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_END_OF_BLOCK:
                    return;

                default:
                    // Skip chunk
                    while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                    break;
            }
        }
    }

    //void stMap::ProcessChunk(FILE * Stream, tChunk * Chunk)
    //{
    //
    //}

    void stMap::ShowInformation()
    {
        printf("Map information\n"
               "===========================================================\n");
        printf("Chipset : %i\n", ChipsetID);
        printf("Size : %ix%i\n", MapWidth, MapHeight);
        printf("Type of Loop : %i\n", TypeOfLoop);
        printf("Use parallax background : %i\n", ParallaxBackground);
        if (ParallaxBackground)
        {
            printf(" -> Background file : %s\n", BackgroundName.c_str());
            printf(" -> Horizontal pan : %i\n", HorizontalPan);
            if (HorizontalPan)
            {
                printf(" --> Horizontal auto pan : %i\n", HorizontalAutoPan);
                if (HorizontalAutoPan)
                    printf(" ---> Horizontal pan speed : %i\n", HorizontalPanSpeed);
            }
            printf(" -> Vertical pan : %i\n", VerticalPan);
            if (VerticalPan)
            {
                printf(" --> Vertical auto pan : %i\n", VerticalAutoPan);
                if (VerticalAutoPan)
                    printf(" ---> Vertical pan speed : %i\n", VerticalPanSpeed);
            }
        }

        if (LowerLayer != NULL)
        {
            printf("\nLower layer map data :\n");
            for (int y = 0; y < MapHeight; y++ )
            {
                for (int x = 0; x < MapWidth; x++ )
                    printf("%04X, ", LowerLayer[x+y*MapWidth]);
                printf("\n");
            }
        }
        if (UpperLayer != NULL)
        {
            printf("\nUpper layer map data :\n");
            for (int y = 0; y < MapHeight; y++ )
            {
                for (int x = 0; x < MapWidth; x++ )
                    printf("%04X, ", UpperLayer[x+y*MapWidth]);
                printf("\n");
            }
        }
        printf("Number of times saved : %i\n", TimesSaved);
    }

    void stMap::Render(SDL_Surface * Destiny, int Layer, int CameraX, int CameraY)
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
        if ( xEnd   > MapWidth ) xEnd   = MapWidth;
        if ( yEnd   > MapHeight) yEnd   = MapHeight;

        switch(Layer)
        {
            case 0:
                // Calculate initial pointer and step
                TilePointer = &LowerLayer[xStart + yStart * MapWidth];
                StepPerY    = MapWidth - (xEnd-xStart);

                // Run through the whole map
                for ( y=yStart; y<yEnd; y++, TilePointer+=StepPerY )
                    for ( x = xStart; x < xEnd; x++, TilePointer++ )
                        Chipset.RenderTile(Destiny, (x<<4)-CameraX, (y<<4)-CameraY, *TilePointer, Frame);
                break;
            case 1:
                // Calculate initial pointer and step
                TilePointer = &UpperLayer[xStart + yStart * MapWidth];
                StepPerY    = MapWidth - (xEnd-xStart);

                // Run through the whole map
                for ( y=yStart; y<yEnd; y++, TilePointer+=StepPerY )
                    for ( x = xStart; x < xEnd; x++, TilePointer++ )
                        Chipset.RenderTile(Destiny, (x<<4)-CameraX, (y<<4)-CameraY, *TilePointer, Frame);
                break;
        }
    }
