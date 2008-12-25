#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_mixer.h>

class Audio 
{
	protected:
		Mix_Music * musica;
		Mix_Chunk * sonido;
		int phaserChannel;
	public:
		void init();
		void musicload(const char* string);
		void stopmusic();
		void soundload(const char* string);
};

#endif
