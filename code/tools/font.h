#ifndef FONT_H_
#define FONT_H_

class Font{
protected:

          int fR,  fG,  fB,  fU; // fU = unused
          const char * Fname;
public:
      int size;
      void init_Font();
      SDL_Surface* drawText(char* string);
      SDL_Surface* drawText(char* string,int r, int b,int g, int u);
      SDL_Surface* drawText(const char* string);
      void draw_temp_Text(SDL_Surface* screen,char* string, int x, int y);
      void Quit();

};

#endif
