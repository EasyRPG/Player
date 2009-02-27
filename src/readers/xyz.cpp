#include <cstdio>
#include "zlib.h"
#include "xyz.h"

SDL_Surface* load_XYZ(const std::string& filename)
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

    file = fopen(filename.c_str(), "rb");
    if (file != NULL)
    {
        char* header;
        header = new char[4];
    	if (header == NULL)
        {   
            printf("Error XYZ Reader: No memory left");
    	    return NULL;
        }
        fread(header, 1, 4, file);
        if (!strcmp(header, "XYZ1"))
        {
            delete header;
            fread(&width, 1, 2, file);
            fread(&height, 1, 2, file);
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, 8, SEEK_SET);
            destSize = 768 + (width * height);
            destBuffer = new unsigned char[destSize];
            buffer = new unsigned char[size - 8];
            if ((buffer == NULL) || (destBuffer == NULL))
            {   
                printf("Error XYZ Reader: No memory left");
                return NULL;
            }
            fread(buffer, 1, size - 8, file);
            fclose(file);
            zlibErrorValue = uncompress((Bytef*)destBuffer, &destSize, (Bytef*)buffer, (uLongf)(size - 8));
            delete buffer;
            surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
    
            SDL_LockSurface(surface);

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
            delete destBuffer;
        }
        else
        {
            perror("XYZ Reader Error: Not a valid XYZ file.");
            delete header;
            fclose(file);
            return NULL;
        }
    }
    else
    {
        perror("XYZ Reader: Error reading file.");
        return NULL;
    }
    SDL_UnlockSurface(surface);
    return surface;
}

