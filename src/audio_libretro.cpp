#ifdef USE_LIBRETRO
#include "audio_libretro.h"

	LibretroAudio::LibretroAudio(){
		
	}
	LibretroAudio::~LibretroAudio(){
		
	}

	void LibretroAudio::BGM_Play(std::string const &, int, int, int){
		
	}
	void LibretroAudio::BGM_Pause(){
		
	}
	void LibretroAudio::BGM_Resume(){
		
	}
	void LibretroAudio::BGM_Stop(){
		
	}
	bool LibretroAudio::BGM_PlayedOnce(){
		return true;
	}
	unsigned LibretroAudio::BGM_GetTicks(){
		return 0;
	}
	void LibretroAudio::BGM_Fade(int){
		
	}
	void LibretroAudio::BGM_Volume(int){
		
	}
	void LibretroAudio::BGM_Pitch(int){
		
	}
	void LibretroAudio::SE_Play(std::string const &, int, int){
		
	}
	void LibretroAudio::SE_Stop(){
		
	}
	void LibretroAudio::Update(){
		if(RenderAudioFrames==0){
			return;
		}
	}

	retro_audio_sample_batch_t LibretroAudio::RenderAudioFrames=0;
	void LibretroAudio::SetRetroAudioCallback(retro_audio_sample_batch_t cb){
		RenderAudioFrames=cb;
	}
	
#endif //USE_LIBRETRO