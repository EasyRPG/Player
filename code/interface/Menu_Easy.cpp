#include "Menu_Easy.h"

void Menu_Easy::init(Audio * theaudio, bool * run,int X,int Y) 
{
	myaudio=theaudio;
	desided=false;
	maxindexY=Y;
	maxindexX=X;
	delay=0;
	running=  run;
}

bool Menu_Easy::desition()
{
	return (desided);
}

void Menu_Easy::restarmenu()
{
	desided=false;
	delay=0;
	indexY=0;
	indexX=0;
	//(*myaudio).freesound();
}

int Menu_Easy::getindexY()
{
	return (indexY);
}

int Menu_Easy::getindexX()
{
	return (indexX);
}

void Menu_Easy::updatekey() 
{

static unsigned char * keyData;
static unsigned char Last_key;
delay++;

if (desided==false)
	if(delay==4)
	{
		keyData = SDL_GetKeyState(NULL);
		if ( keyData[SDLK_ESCAPE] )
		{
			* running = false;
		}

		if ( keyData[SDLK_LEFT]  ) 
		{
			indexX--;
			//(*myaudio).soundload("../Sound/Cursor1.wav");
		}

		if ( keyData[SDLK_RIGHT] ) 
		{
			indexX++;
  			// (*myaudio).soundload("../Sound/Cursor1.wav");
		}

		if ( keyData[SDLK_UP] ) 
		{
			indexY--;
			// (*myaudio).soundload("../Sound/Cursor1.wav");
		}

		if ( keyData[SDLK_DOWN]  ) 
		{
			indexY++;
			// (*myaudio).soundload("../Sound/Cursor1.wav");
		}

		if (( keyData[LMK_Z]  )&&(Last_key!=LMK_Z)) 
		{
 			//  (*myaudio).soundload("../Sound/Decision2.wav");
			Last_key=LMK_Z;
			desided= true;
		}

		if ((!keyData[LMK_Z])&&(Last_key==LMK_Z))
		{
			Last_key=0;
		}

		if (indexX<0)
		{
			indexX=maxindexX;
		}
		
		if (indexX>maxindexX)
		{
			indexX=0;
		}

		if (indexY<0)
		{
			indexY=maxindexY;
		}
		
		if (indexY>maxindexY)
		{
		indexY=0;
		}
		delay=0;
	}
}

