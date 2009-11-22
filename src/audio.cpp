#include "audio.h"

bool Audio::initialize()
{
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            _fatal_error(SDL_GetError());
            return false;
        }
    }

    int flags = 0;    
    flags = Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

    if ( !(flags & MIX_INIT_MP3) ) {
        _fatal_error(Mix_GetError());
        return false;
    }

    if ( !(flags & MIX_INIT_OGG) ) {
        _fatal_error(Mix_GetError());
        return false;
    }

    return true;
}

void Audio::bgm_play(std::string filename)
{
	
}

void Audio::bgm_play(std::string filename, int volume)
{
	
}

void Audio::bgm_play(std::string filename, int volume, int pitch)
{
	
}

void Audio::bgm_stop()
{
	
}

void Audio::bgm_fade(int time)
{
	
}

void Audio::bgs_play(std::string filename)
{
	
}

void Audio::bgs_play(std::string filename, int volume)
{
	
}

void Audio::bgs_play(std::string filename, int volume, int pitch)
{
	
}

void Audio::bgs_stop()
{
	
}

void Audio::bgs_fade(int time)
{
	
}

void Audio::me_play(std::string filename)
{
	
}

void Audio::me_play(std::string filename, int volume)
{
	
}

void Audio::me_play(std::string filename, int volume, int pitch)
{
	
}

void Audio::me_stop()
{
	
}

void Audio::me_fade(int time)
{
	
}

void Audio::se_play(std::string filename)
{
	
}


void Audio::se_play(std::string filename, int volume)
{
	
}


void Audio::se_play(std::string filename, int volume, int pitch)
{
	
}

void Audio::se_stop()
{
	
}
