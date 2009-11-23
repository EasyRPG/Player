/* tools.h, miscellaneous shared routines.
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

#ifndef TOOLS_H
#define TOOLS_H

#define PNG 0
#define BMP 1
#define XYZ 2
#define MID 3
#define WAV 4
#define MP3 5

    // =========================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include "SDL.h"
    // =========================================================================
#ifdef WIN32 
    #include <windows.h>
#endif

    #include "zlib.h"
    // --- Chunk structure -----------------------------------------------------
    struct tChunk
    {
        unsigned char ID;

        union
        {
        	unsigned long Length;
        	unsigned long NumEvents;
        };

        unsigned long BytesRead;
    };

    // --- Read functions ------------------------------------------------------
    unsigned long ReadCompressedInteger(FILE * FileStream);
    unsigned long ReadCompressedIntegerCount(FILE * FileStream);
    int CountRead(int data);
    std::string ReadString(FILE * FileStream);
    std::string ReadString(FILE * FileStream, unsigned char Lenght);

    // --- Surface Tools -------------------------------------------------------
    void SetTransparent(SDL_Surface * ima);
    SDL_Surface * CreateSurface(int Width, int Height);
    SDL_Surface * LoadSurface(std::string Filename);
    SDL_Surface * GrabFromSurface(SDL_Surface * Source, int sX, int sY, int sW, int sH);
    void DrawSurface(SDL_Surface * Destiny, int dX, int dY, SDL_Surface * Source, int sX=0, int sY=0, int sW=-1, int sH=-1);
    SDL_Surface* load_XYZ(const std::string& filename);

    // --- Miscellaneous functions ---------------------------------------------
    void s_toupper(char *dest, char *s);
#ifdef WIN32
    int get_img_extension(std::string& fname);
    int get_mus_extension(std::string& fname);
#else
    char * case_insensitive_and_format_img_exist(const char *directory, std::string & file);
	char * case_insensitive_and_format_msc_exist(const char *directory, std::string & file);
	int case_insensitive_exist( std::string & dir_file, const char *directory, char *file);
#endif
    void _fatal_error(const char *perr);
    

#endif
