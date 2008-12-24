

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "font.h"
void Font::init_Font()//esto es asi porque no se me ocurre aun algo mejor
{	Fname="../Fonts/BASKVILL.TTF";
    size=14;
    fR=255;
    fG=255;
    fB=255;
    fU=0; //unused
    TTF_Init();
}    

SDL_Surface* Font::drawText(char* string)
{
   TTF_Font* font = TTF_OpenFont(Fname, size);
   SDL_Color foregroundColor = { fR, fG, fB, fU};  
   SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
   TTF_CloseFont(font); 
   return(textSurface); 
}

void Font::draw_temp_Text(SDL_Surface* screen,char* string, int x, int y)
{
   TTF_Font* font = TTF_OpenFont(Fname, size);
   SDL_Color foregroundColor = { fR, fG, fB, fU };  
   SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
   SDL_Rect textLocation = { x, y, 0, 0 };
   SDL_BlitSurface(textSurface, NULL, screen, &textLocation);
   SDL_FreeSurface(textSurface);
   TTF_CloseFont(font);
   
}
SDL_Surface* Font::drawText(const char* string)
{
   TTF_Font* font = TTF_OpenFont(Fname, size);
   SDL_Color foregroundColor = { fR, fG, fB, fU };  
   SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
   TTF_CloseFont(font);
   return(textSurface); 
   
}
SDL_Surface* Font::drawText(char* string,int r, int b,int g, int u)
{
   TTF_Font* font = TTF_OpenFont(Fname, size);
   SDL_Color foregroundColor = { r, g, b, u };
   SDL_Surface* textSurface = TTF_RenderText_Blended(font, string,foregroundColor);
   TTF_CloseFont(font);
   return(textSurface); 
}
void Font::Quit()//esto es asi porque no se me ocurre aun algo mejor
{
TTF_Quit();
}
