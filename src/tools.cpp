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
    #include <string.h>
    #include <dirent.h>
    #include <ctype.h>
    #include <iostream>
    #include "SDL.h"
//    #include "SDL_image.h"
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
            bool return_value;
            return_value = fread(&Temporal, sizeof(char), 1, FileStream);

            // Check if it's a BER integer
            Value |= Temporal&0x7F;

        } while (Temporal&0x80);

        return Value;
    }

      int CountRead(int data)//por esto tools deveria ser una clase
       {static int count;
       if(data != -1)
       count=data;
       return (count);
       }

         unsigned long ReadCompressedIntegerCount(FILE * FileStream)
        {
            unsigned long Value = 0;
            unsigned char Temporal = 0;
            int sizeread=0;
            // int i = 0;
            do
            {
                Value <<= 7;

                // Get byte's value
                bool return_value;
                return_value = fread(&Temporal, sizeof(char), 1, FileStream);
                sizeread++;
                // Check if it's a BER integer
                Value |= Temporal&0x7F;

            } while (Temporal&0x80);
            CountRead (sizeread);
            return Value;
        }


    string ReadString(FILE * FileStream)
    {
        unsigned char Length;
        char        * Characters;
        string        String;

        // Read string lenght's
        bool return_value;
        return_value = fread(&Length, sizeof(char), 1, FileStream);
        if (Length == 0) return string("");

        // Allocate string buffer
        Characters = new char[Length+1];
        memset(Characters, 0, Length+1);
        return_value = fread(Characters, sizeof(char), Length, FileStream);

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
        bool return_value;
        return_value = fread(Characters, sizeof(char), Length, FileStream);

        // Get string and free characters buffer
        String = string(Characters);
        delete Characters;

        return String;
    }


void ModRGB(SDL_Surface * img, int red,int green,int blue)
{
    SDL_Color * color;
    int maxColors = img->format->palette->ncolors;
    for (int i = 1; i < maxColors; i ++)
    {
        color = & img->format->palette->colors[i];

	color->r+=red;
	color->g+=green;
	color->b+=blue;


    }
	SetTransparent(img);
}

void SetTransparent(SDL_Surface * ima)
{
    /* rm2k uses palette index 0 for transparent color instead of colorkey */
    SDL_Color colorkey = ima->format->palette->colors[0];
    SDL_Color * color;
    int maxColors = ima->format->palette->ncolors;
    for (int i = 1; i < maxColors; i ++)
    {
        /* Workaround to set transparency only in index 0*/
        color = & ima->format->palette->colors[i];


        if (colorkey.r == color->r && \
            colorkey.g == color->g && \
            colorkey.b == color->b)
        {
            if (color->b > 255 - 8)
                color->b -= 8;
            else
                color->b += 8;
        }
    } /* Thanks to Hugo Ruscitti (www.losersjuegos.com.ar) for this fix */
}

    SDL_Surface * CreateSurface(int Width, int Height)
    {
        SDL_Surface * dummySurface = NULL;
        SDL_Surface * realSurface = NULL;
        SDL_Color color;
        Uint32 colorKey;

        dummySurface = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, Width, Height, 8, 0, 0, 0, 0);
        if (!dummySurface) return NULL;

        SetTransparent(dummySurface);

        realSurface  = SDL_DisplayFormat(dummySurface);
        if ( !realSurface ) return NULL;

        color = dummySurface->format->palette->colors[0];
        colorKey = SDL_MapRGB(realSurface->format, color.r, color.g, color.b);
        SDL_SetColorKey(realSurface, SDL_SRCCOLORKEY, colorKey);

        SDL_FreeSurface(dummySurface);

        if ( !realSurface ) return NULL;
        SDL_FillRect(realSurface, NULL, 0);

        return realSurface;

    }

    SDL_Surface * LoadSurface(string Filename)
    {
        SDL_Surface * dummySurface = NULL;
        SDL_Surface * realSurface = NULL;
        SDL_Color color;
        Uint32 colorKey;

 //       dummySurface = IMG_Load(Filename.c_str());
        if (dummySurface == NULL)
        {
        std::cerr << "Error: Unable to open file: " << Filename <<  std::endl;
        exit(1);
        }
        if (!dummySurface) return NULL;

        SetTransparent(dummySurface);

        realSurface  = SDL_DisplayFormat(dummySurface);
        if ( !realSurface ) return NULL;

        color = dummySurface->format->palette->colors[0];
        colorKey = SDL_MapRGB(realSurface->format, color.r, color.g, color.b);
        SDL_SetColorKey(realSurface, SDL_SRCCOLORKEY, colorKey);

        SDL_FreeSurface(dummySurface);

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
        SDL_Surface * Return = CreateSurface(16, 16);
        DrawSurface(Return, 0, 0, Source, sX, sY, sW, sH);

        return Return;
    }

/* Converts a string s into upper-case string dest. Need of <ctype.h> */
/* Warning : you must malloc "dest" ! */
void s_toupper(char *dest, char *s)
{
    unsigned int i;

    for (i=0;i<=strlen(s);i++)
    {
        dest[i]=(char)toupper((int)s[i]);
    }
}
char * case_insensitive_and_format_msc_exist(const char *directory, string & file)
{
//		char * return_name;
		static std::string img_string;
		static std::string file_ext;
		img_string.clear();
		img_string.clear();
		file_ext.append(file);
		file_ext.append(".wav");
		if(case_insensitive_exist(img_string, directory, (char*) file_ext.c_str()))
		{
			return ((char*)img_string.c_str());
		}
		file_ext.clear();
		file_ext.append(file);
		file_ext.append(".mid");
		if(case_insensitive_exist(img_string, directory,(char*) file_ext.c_str()))
		{
			return ((char*)img_string.c_str());
		}
		file_ext.clear();
		file_ext.append(file);
		file_ext.append(".mp3");
		if(case_insensitive_exist(img_string, directory,(char*) file_ext.c_str()))
		{
			return ((char*)img_string.c_str());
		}
        return ((char*)file.c_str());
}
char * case_insensitive_and_format_img_exist(const char *directory, string & file)
{
//		char * return_name;
		static std::string img_string;
		static std::string file_ext;
		img_string.clear();

		file_ext.clear();
		file_ext.append(file);
		file_ext.append(".png");
		if(case_insensitive_exist(img_string, directory, (char*) file_ext.c_str()))
		{
			return ((char*)img_string.c_str());
		}
		file_ext.clear();
		file_ext.append(file);
		file_ext.append(".bmp");
		if(case_insensitive_exist(img_string, directory,(char*) file_ext.c_str()))
		{
			return ((char*)img_string.c_str());
		}
		file_ext.clear();
		file_ext.append(file);
		file_ext.append(".xyz");
		if(case_insensitive_exist(img_string, directory,(char*) file_ext.c_str()))
		{
			return ((char*)img_string.c_str());
		}
        return ((char*)file.c_str());
}

/* gives "filename" that exists in "directory" with CASE INSENSITIVE TEST*/
/* in : "file", "directory". out :"return-name" (must be malloc'd) */
/* return : 1 if filename exists (CASE INSENSITIVE), 0 if not */

int case_insensitive_exist( string & dir_file, const char *directory, char *file)
{
    int exist = 0;
    struct dirent *d_ent;
    DIR *dp;

    /* open directory*/
    dp=opendir(directory);
    if (dp)
    {
        char upper_d_name[256];
        char upper_file[256];
        while ((d_ent=readdir(dp)))
        {
            s_toupper(upper_d_name,d_ent->d_name);
            s_toupper(upper_file, file);
            if (!strcmp(upper_d_name,upper_file))
            {
                exist=1;
                break;
            }
        }
    }
    if (exist)
    {
        dir_file.append(directory);
        dir_file.append(d_ent->d_name);
		return 1;
    }
    else
    {
        return 0;
    }

}
