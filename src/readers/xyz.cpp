#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include "SDL.h"

int main()
{
    FILE *file;
    unsigned char *buffer;
    unsigned char *destBuffer;
    unsigned int size;
    uLongf destSize;
    unsigned short width;
    unsigned short height;
    int zlibErrorValue;
    SDL_Surface *surface;
    SDL_Palette *palette;
    int i;
    char *pixel;
    
    file = fopen("Basis.xyz", "rb");
    if (file != NULL)
    {
        buffer = new unsigned char[4];
        fread(buffer, 1, 4, file);
        if (!strcmp((const char *)buffer, "XYZ1"))
        {
            delete buffer;
            fread(&width, 1, 2, file);
            fread(&height, 1, 2, file);
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, 8, SEEK_SET);
            destSize = 768 + (width * height);
            destBuffer = new unsigned char[destSize];
            buffer = new unsigned char[size - 8];
            fread(buffer, 1, size - 8, file);
            fclose(file);
            zlibErrorValue = uncompress((Bytef*)destBuffer, &destSize, (Bytef*)buffer, (uLongf)(size - 8));
            delete buffer;
            surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
            palette = (surface->format)->palette;
            for (i = 0; i < 256; i++)
            {
                palette->colors[i].r = destBuffer[i * 3];
                palette->colors[i].g = destBuffer[i * 3 + 1];
                palette->colors[i].b = destBuffer[i * 3 + 2];
            }
            pixel = (char*)surface->pixels;
            for (i = 0; i < (width * height); i++)
            {
                pixel[i] = destBuffer[i + 768];
            }
            SDL_SaveBMP(surface, "Basis.bmp");
            SDL_FreeSurface(surface);
        }
        else
        {
            perror("Error: Not a valid XYZ file.");
            delete buffer;
            fclose(file);
            return 2;
        }
    }
    else
    {
        perror("Error reading file.");
        return 1;
    }
    return 0;
}

