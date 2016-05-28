#ifdef USE_LIBRETRO
#include "audio_libretro.h"
	//For now LibretroAudio is empty - no included mixer supports hijacking the mixed output audio.
	//TODO: Find suitble solution or write own mixer
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

	}
	
	void LibretroAudio::EnableAudio(bool enabled){
		//Till now libretro has no influence on the audio
	}
	
	void LibretroAudio::AudioThreadCallback(){
		if(RenderAudioFrames==0){
			return;
		}
	}

	retro_audio_sample_batch_t LibretroAudio::RenderAudioFrames=0;
	void LibretroAudio::SetRetroAudioCallback(retro_audio_sample_batch_t cb){
		RenderAudioFrames=cb;
	}
			
	unsigned LibretroAudio::output_samplerate=0;
	void LibretroAudio::SetOutputSampleRate(unsigned samplerate){
		output_samplerate=samplerate;
	}
	
	unsigned LibretroAudio::samples_per_frame=0;
	void LibretroAudio::SetNumberOfSamplesPerFrame(unsigned samples){
		samples_per_frame=samples;
	}
	
#endif //USE_LIBRETRO