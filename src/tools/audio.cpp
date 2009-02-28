#include <string>
#include <iostream>
#include "audio.h"

bool Audio::wasInit = false;

const int Audio::AUDIO_RATE = MIX_DEFAULT_FREQUENCY; // 22050
const Uint16 Audio::AUDIO_FORMAT = MIX_DEFAULT_FORMAT; // AUDIO_S16SYS
const int Audio::AUDIO_CHANNELS = 2; // 1.- mono  2.- stereo
const int Audio::AUDIO_BUFFER = 4096; // default

bool Audio::init()
{
    if (wasInit)
    {
        std::cerr << "*** Warning: Audio was already initialized." << std::endl;
        return false;
    }

    if (Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFER) < 0)
    {
        std::cerr << "Error: " << Mix_GetError() << std::endl;
        return false;
    }

    //phaserChannel = -1; // Var left to revise
    wasInit = true;

    return true;
}
/*
void Audio::init()
{
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16;
  int audio_channels = 2;
  int audio_buffers = 4096;

  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    printf("Unable to open audio!\n");
    exit(1);
  }
 phaserChannel = -1;
}*/

/* === Music Class Implementation === */

Music::Music()
{
    music = NULL;
}

Music::~Music()
{
    Mix_FreeMusic(music);

}


bool Music::load(const char* musicf)
{
    if (music == NULL)
    {
        music = Mix_LoadMUS(musicf);
    }
    else
    {
        Mix_FreeMusic(music);
        music = Mix_LoadMUS(musicf);
     //   std::cerr << "Error: Cannot load: " << musicf << "  Music already loaded" << std::endl;
    }
    actual_music.clear();
    actual_music.append(musicf);

    if (music == NULL)
    {
        std::cerr << "Error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

bool Music::play(int loops)
{
    if (music == NULL)
    {
        std::cerr << "Error: Cannot play music. Please load it first." << std::endl;
        return false;
    }

    if (Mix_PlayMusic(music, loops) < 0)
    {
        std::cerr << "Error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

bool Music::play(int loops, int ms)
{
    if (music == NULL)
    {
        std::cerr << "Error: Cannot play music. Please load it first." << std::endl;
        return false;
    }

    if (ms < 0)
    {
        std::cerr << "*** Warning: Negative time ms: " << ms << std::endl;
    }

    if (Mix_FadeInMusic(music, loops, ms) < 0)
    {
        std::cerr << "Error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}


void Music::stop()
{
    if (!Mix_PlayingMusic())
    {
        std::cerr << "*** Warning: Stopping music not being played." << std::endl;
    }
	Mix_HaltMusic();
}

void Music::setVolume(int vol)
{
    if (vol > MIX_MAX_VOLUME)
    {
        std::cerr << "*** Warning: Setting Volume to: " << vol << " Max is" << MIX_MAX_VOLUME << std::endl;
    }

    if (vol < 0)
    {
        std::cerr << "*** Warning: Setting Volume to: " << vol << " Volume will not be set" << std::endl;
    }
    Mix_VolumeMusic(vol);
}

void Music::pause_resume()
{
    if (Mix_PausedMusic())
    {
        Mix_ResumeMusic();
    }
    else
    {
        if (!Mix_PlayingMusic())
        {
            std::cerr << "Warning: Pausing music not being played. " << std::endl;
        }
        Mix_PauseMusic();
    }
}

/* === Sound Class Implementation === */

int Sound::nSounds = 0;

Sound::Sound()
{
    actual_music="";
    actualChannel = 0;
    sound = NULL;
}

Sound::~Sound()
{
    nSounds--;
    if (Mix_Playing(actualChannel))
    {
        std::cerr << "*** Warning: Playing sound freed. It should have been stopped before." << std::endl;
    }
    Mix_FreeChunk(sound);
}

bool Sound::load(const char* soundf)
{

if(actual_music.compare(soundf))
 {

if(sound != NULL)
{
    nSounds--;
    if (Mix_Playing(actualChannel))
    {
        std::cerr << "*** Warning: Playing sound freed. It should have been stopped before." << std::endl;
    }
    Mix_FreeChunk(sound);

}

    actual_music.clear();
    actual_music.append(soundf);

    sound = Mix_LoadWAV(soundf);
    if (sound == NULL)
    {
        std::cerr << "Error: " << Mix_GetError() << std::endl;
        return false;
    }

    nSounds++;
    if(Mix_AllocateChannels(-1) < nSounds)
    {
        Mix_AllocateChannels(nSounds);
    }

    actualChannel = nSounds;
 }
    return true;
}

bool Sound::play(int loops)
{
    if (sound == NULL)
    {
        std::cerr << "Error: Cannot play sound. Please load it first." << std::endl;
        return false;
    }
    if (Mix_PlayChannel(actualChannel, sound, loops) < 0)
    {
        std::cerr << "Error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;

}

bool Sound::play(int loops, int ms)
{

    if (sound == NULL)
    {
        std::cerr << "Error: Cannot play music. Please load it first." << std::endl;
        return false;
    }

    if (ms < 0)
    {
        std::cerr << "*** Warning: Negative time ms: " << ms << std::endl;
    }

    if (Mix_FadeInChannel(actualChannel, sound, loops, ms) < 0)
    {
        std::cerr << "Error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

void Sound::stop()
{
    if (Mix_Playing(actualChannel) == 0)
    {
        std::cerr << "*** Warning: Channel " << actualChannel << " is not being played." << std::endl;
    }
    Mix_HaltChannel(actualChannel);
}

void Sound::setVolume(int vol)
{
    if (vol > MIX_MAX_VOLUME)
    {
        std::cerr << "*** Warning: Setting Volume to: " << vol << " Max is" << MIX_MAX_VOLUME << std::endl;
    }

    if (vol < 0)
    {
        std::cerr << "*** Warning: Setting Volume to: " << vol << " Volume will not be set" << std::endl;
    }
    Mix_Volume(actualChannel, vol);
}

void Sound::pause_resume()
{
    if (Mix_Paused(actualChannel))
    {
        Mix_Resume(actualChannel);
    }
    else
    {
        if (!Mix_Playing(actualChannel))
        {
            std::cerr << "Warning: Pausing sound not being played. " << std::endl;
        }
        Mix_Pause(actualChannel);
    }
}


