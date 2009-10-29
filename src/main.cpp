#include <stdio.h>
#include "SDL.h"

#include "main_data.h"
#include "graphics.h"
#include "scene_title.h"

int main(int argc, char **argv)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
    {
        // Error
    }
	
	// Disable Mouse Cursor
	SDL_ShowCursor(SDL_DISABLE);
	
	// Start Graphics
	Graphics::initialize();
	
	// Create Scene Title
	Main_Data::scene = new Scene_Title();
	
	// Main loop
	while(Main_Data::scene_type != "")
	{
		Main_Data::scene->main();
	}
	
	// Quit SDL
	SDL_Quit();
	
	return 0;
}
