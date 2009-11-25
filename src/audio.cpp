#include "audio.h"

namespace {
    Mix_Music *bgm = NULL;
}

bool Audio::initialize()
{
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            _fatal_error(SDL_GetError());
            return false;
        }
    }

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        _fatal_error(Mix_GetError());
        return false;
    }
#if (SDL_MIXER_MAJOR_VERSION > 1) || ((SDL_MIXER_MINOR_VERSION == 1 ) && \
	(SDL_MIXER_MINOR_VERSION >= 2) && (SDL_MIXER_PATCHLEVEL >= 10))
    int flags = 0;    
    flags = Mix_Init(MIX_INIT_MP3);

    if ( !(flags & MIX_INIT_MP3) ) {
        _fatal_error(Mix_GetError());
        return false;
    }
#endif
    return true;
}

/* Loads filename in memory and plays it. Returns NULL on errors. */
Mix_Music* Audio::bgm_play(std::string filename)
{
    std::string serr;
    int ret = 0;
#ifdef WIN32
    int file_ext;
    file_ext = get_mus_extension(filename);
    switch (file_ext) {
        case MID:
        case WAV:
        case MP3:
            bgm = Mix_LoadMUS(filename.c_str());
            break;
        default:
            serr = "Couldn't open ";
            serr.append(filename);
            _fatal_error(serr.c_str());
            return NULL;
    }
#else
    // TODO Implement file extension guessing for non WIN32 systems
    filename.append(".mid");
    bgm = Mix_LoadMUS(filename.c_str());
#endif

    if (bgm == NULL) {
        _fatal_error(Mix_GetError());
        return NULL;
    } else {
        ret = Mix_PlayMusic(bgm, -1);
        if (ret < 0) {
            _fatal_error(Mix_GetError());
            return NULL;
        }
    }
    return bgm;    
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
