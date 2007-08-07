/* tools.cpp, miscellaneous shared routines.
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
    #include "SDL_image.h"
    #include "tools.h"
    using namespace std;
    // =========================================================================
    
    unsigned long ReadCompressedInteger(FILE * FileStream)
    {
        unsigned long Value = 0;
        unsigned char Temporal = 0;
        
        // int i = 0;
        do
        {
            Value <<= 7;
            
            // Get byte's value
            fread(&Temporal, sizeof(char), 1, FileStream);
            
            // Check if it's a BER integer
            Value |= Temporal&0x7F;
            
        } while (Temporal&0x80);
        
        return Value;
    }
    
    string ReadString(FILE * FileStream)
    {
        unsigned char Length;
        char        * Characters;
        string        String;
        
        // Read string lenght's
        fread(&Length, sizeof(char), 1, FileStream);
        if (Length == 0) return string("");
        
        // Allocate string buffer
        Characters = new char[Length+1];
        memset(Characters, 0, Length+1);
        fread(Characters, sizeof(char), Length, FileStream);
        
        // Get string and free characters buffer
        String = string(Characters);
        delete Characters;
        
        return String;
    }

    string ReadString(FILE * FileStream, unsigned char Length)
    {
        char        * Characters;
        string        String;
        
        // Allocate string buffer
        Characters = new char[Length+1];
        memset(Characters, 0, Length+1);
        fread(Characters, sizeof(char), Length, FileStream);
        
        // Get string and free characters buffer
        String = string(Characters);
        delete Characters;
        
        return String;
    }

    SDL_Surface * CreateSurface(int Width, int Height, int)
    {
        SDL_Surface * dummySurface = NULL;
        SDL_Surface * realSurface = NULL;
        
        dummySurface = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, Width, Height, 32, 0, 0, 0, 0);
        if (!dummySurface) return NULL;
        
        realSurface  = SDL_DisplayFormat(dummySurface);
        SDL_FreeSurface(dummySurface);
        if ( !realSurface ) return NULL;
        
        SDL_SetColorKey(realSurface, realSurface->flags|SDL_SRCCOLORKEY, 0);
        SDL_FillRect(realSurface, NULL, 0);
        
        return realSurface;
    }
    
    SDL_Surface * LoadSurface(string Filename)
    {       
        SDL_Surface * dummySurface = NULL;
        SDL_Surface * realSurface = NULL;
        
        dummySurface = IMG_Load(Filename.c_str());
        if (!dummySurface) return NULL;
        
        realSurface  = SDL_DisplayFormat(dummySurface);
        SDL_FreeSurface(dummySurface);
        if ( !realSurface ) return NULL;
        
        SDL_SetColorKey(realSurface, SDL_SRCCOLORKEY, 1);
        
        return realSurface;
    }
    
    void DrawSurface(SDL_Surface * Destiny, int dX, int dY, SDL_Surface * Source, int sX, int sY, int sW, int sH)
    {
        if (sW == -1) sW = Source->w;
        if (sH == -1) sH = Source->h;
        
        SDL_Rect sourceRect;
        sourceRect.x = sX;
        sourceRect.y = sY;
        sourceRect.w = sW;
        sourceRect.h = sH;
        
        SDL_Rect destinyRect;
        destinyRect.x = dX;
        destinyRect.y = dY;
        destinyRect.w = sW;
        destinyRect.h = sH;
        
        SDL_BlitSurface(Source, &sourceRect, Destiny, &destinyRect);
    }

    SDL_Surface * GrabFromSurface(SDL_Surface * Source, int sX, int sY, int sW, int sH)
    {
        SDL_Surface * Return = CreateSurface(16, 16, 0);
        DrawSurface(Return, 0, 0, Source, sX, sY, sW, sH);
        
        return Return;
    }
