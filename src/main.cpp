#include <stdio.h>
#include "SDL.h"

#include "main_data.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "scene_title.h"

int main(int argc, char **argv)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        // Error
    }
#ifdef _DEBUG
    SDL_InitSubSystem(SDL_INIT_NOPARACHUTE);
#endif

	// Disable Mouse Cursor
	SDL_ShowCursor(SDL_DISABLE);
	
	// Start Graphics
	Graphics::initialize();

    Input::initialize();

    if (!Audio::initialize()) {
        _fatal_error("Couldn't initialise audio!");
    }
	
	// Create Scene Title
	Main_Data::scene = new Scene_Title();
	
	// Main loop
	while (Main_Data::scene_type != SCENE_NULL)
	{
		Main_Data::scene->main_function();
	}
	
	// Quit SDL
	SDL_Quit();
	
	return 0;
}
