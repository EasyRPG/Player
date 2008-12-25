#include "audio.h"


void Audio::init()
{
	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16;
	int audio_channels = 2;
	int audio_buffers = 4096;
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) 
	{
		printf("Unable to open audio!\n");
		exit(1);
	}
	phaserChannel = -1;
}

void Audio::musicload(const char* string)
{
	musica = Mix_LoadMUS(string);
	Mix_PlayMusic(musica, -1);
	//Mix_HookMusicFinished(funcion); //por is queremos llamar uan funcion cuando termine
}

void Audio::stopmusic()
{
	Mix_HaltMusic();
	Mix_FreeMusic(musica);
	musica = NULL;
}

void Audio::soundload(const char* string)
{
	/* sonido = Mix_LoadWAV(string);
	if(phaserChannel < 0) 
	{
		phaserChannel = Mix_PlayChannel(-1, sonido, 1);
	}
	else 
	{
		Mix_HaltChannel(phaserChannel);
		phaserChannel = -1;
	}*/
}
