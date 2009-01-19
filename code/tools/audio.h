#ifndef AUDIO_H_
#define AUDIO_H_
#include "SDL.h"
#include "SDL_mixer.h"

class Audio
{
    private:
        static const int AUDIO_RATE,
                         AUDIO_CHANNELS,
                         AUDIO_BUFFER;
        static const Uint16 AUDIO_FORMAT; // = AUDIO_S16

        static bool wasInit;

    //protected:
        //int phaserChannel;

    public:
        static bool init();

        virtual bool load(const char*) = 0;
        virtual bool play(int) = 0;
        virtual bool play(int, int) = 0;
        virtual void stop() = 0;
        virtual void setVolume(int) = 0;
        virtual void pause_resume() = 0;
};

/* Classes Music and Sound have exactly the same methods.
   However Music is the choice to play music, and
   Sound is the choice to play small sounds.
   You can have more than one sound playing at a time,
   but this does not apply to music. */

class Music: public Audio
{
    private:
        Mix_Music * music;

    public:

        Music();
        ~Music();

        bool load(const char* musicf);
        bool play(int loops); // -1 for infinite loops
        bool play(int loops, int ms); // Fade In in miliseconds
        void stop();
        void setVolume(int vol);
        void pause_resume();

};

class Sound: public Audio
{
    private:
        static int nSounds;
        int actualChannel;
        Mix_Chunk *sound;
    public:

        Sound();
        ~Sound();

        bool load(const char* soundf);
        bool play(int loops); // -1 for infinite loops
        bool play(int loops, int ms); // Fade In in miliseconds
        void stop();
        void setVolume(int vol);
        void pause_resume();
};
#endif
