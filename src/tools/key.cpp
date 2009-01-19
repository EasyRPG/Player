#include <SDL/SDL.h>

bool Key_press_and_realsed( unsigned char key )
{
	unsigned char * keyData;
	keyData = SDL_GetKeyState(NULL);
	static unsigned char Last_key;
        if (( keyData[key]  )&&(Last_key!=key)) 
	{
		Last_key=key;
		return(true);
	}
        if ((!keyData[key])&&(Last_key==key))
	{
		Last_key=0; 
        } 
	return(false);
}
