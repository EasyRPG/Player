#ifdef USE_LIBRETRO
#include "audio_libretro.h"
#include "filefinder.h"
#include "output.h"
#include <rthreads/rthreads.h>

	static slock_t * mutex=0;

		LibretroAudio::Channel LibretroAudio::BGM_Channels[nr_of_bgm_channels];
		LibretroAudio::Channel LibretroAudio::SE_Channels[nr_of_se_channels];
		bool LibretroAudio::BGM_PlayedOnceIndicator;
		bool LibretroAudio::Muted=true;


	LibretroAudio::LibretroAudio(){
		if(mutex==0){
			mutex=slock_new();
		}
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
			BGM_Channels[i].decoder.reset();
		}
		for(unsigned i=0;i<nr_of_se_channels;i++){
			SE_Channels[i].decoder.reset();
		}
		BGM_PlayedOnceIndicator=false;
	}
	LibretroAudio::~LibretroAudio(){
		if(mutex!=0){
			slock_free(mutex);
			mutex=0;
		}
	}

	void LibretroAudio::BGM_Play(std::string const & file, int volume, int pitch, int fadein){
		if(Muted) return;
		bool bgm_set=false;
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
			BGM_Channels[i].stopped=true; //Stop all running background music
			if(!BGM_Channels[i].decoder&&!bgm_set){
				//If there is an unused bgm channel
				bgm_set=true;
				slock_lock(mutex);
				BGM_PlayedOnceIndicator=false;
				slock_unlock(mutex);
				PlayOnChannel(BGM_Channels[i],file,volume,pitch,fadein);
			}
		}
	}
	void LibretroAudio::BGM_Pause(){
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
		BGM_Channels[i].paused=true;
		}
	}
	void LibretroAudio::BGM_Resume(){
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
		BGM_Channels[i].paused=false;
		}
	}
	void LibretroAudio::BGM_Stop(){
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
			BGM_Channels[i].stopped=true; //Stop all running background music
		}
	}
	bool LibretroAudio::BGM_PlayedOnce(){
		return BGM_PlayedOnceIndicator;
	}
	unsigned LibretroAudio::BGM_GetTicks(){
		unsigned ticks=0;
		slock_lock(mutex);
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
			if(BGM_Channels[i].decoder){
				ticks=BGM_Channels[i].decoder->GetTicks();
			}
		}
		slock_unlock(mutex);
		return ticks;
	}
	void LibretroAudio::BGM_Fade(int fade){
		slock_lock(mutex);
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
			if(BGM_Channels[i].decoder){
				BGM_Channels[i].decoder->SetFade(BGM_Channels[i].decoder->GetVolume(),0,fade);
			}
		}
		slock_unlock(mutex);
	}
	void LibretroAudio::BGM_Volume(int volume){
		slock_lock(mutex);
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
			if(BGM_Channels[i].decoder){
				BGM_Channels[i].decoder->SetVolume(volume);
			}
		}	
		slock_unlock(mutex);
	}
	void LibretroAudio::BGM_Pitch(int pitch){
		slock_lock(mutex);
		for(unsigned i=0;i<nr_of_bgm_channels;i++){
			if (BGM_Channels[i].decoder->GetType() == "midi") {
				//Midi handles the pitching without actually changing the pitch (just the tempo)
				BGM_Channels[i].decoder->SetPitch(pitch);
			} else if(BGM_Channels[i].decoder){
				unsigned divider=ggt(output_samplerate*100,BGM_Channels[i].samplerate*pitch);
				BGM_Channels[i].upsample=(output_samplerate*100)/divider;
				BGM_Channels[i].downsample=(BGM_Channels[i].samplerate*pitch)/divider;
			}
		}
		slock_unlock(mutex);
	}
	void LibretroAudio::SE_Play(std::string const &, int, int){
		
	}
	void LibretroAudio::SE_Stop(){
		
	}
	void LibretroAudio::Update(){

	}
	
	bool LibretroAudio::PlayOnChannel(Channel & chan,std::string const & file, int volume, int pitch, int fadein){
		chan.paused=true; //Pause channel so the audio thread doesn't work on it
		chan.stopped=false; //Unstop channel so the audio thread doesn't delete it
		std::string const path = FileFinder::FindMusic(file);
		if (path.empty()) {
			Output::Debug("Music not found: %s", file.c_str());
			return false;
		}

		FILE* filehandle = FileFinder::fopenUTF8(path, "rb");
		if (!filehandle) {
			Output::Warning("Music not readable: %s", file.c_str());
			return false;
		}

		chan.decoder = AudioDecoder::Create(filehandle, path);
		if (chan.decoder && chan.decoder->Open(filehandle)) {
			int samplerate=output_samplerate;
			AudioDecoder::Format sampleformat=output_sample_format;
			int channels=output_channels;
			if (chan.decoder->GetType() == "midi") {
				// Turn dowm midi sample rate
				samplerate /= 2;
				
				//Midi handles the pitching without actually changing the pitch (just the tempo)
				chan.decoder->SetPitch(pitch);
				pitch=100;
			}
			chan.decoder->SetFormat(samplerate, sampleformat, channels);
			chan.decoder->GetFormat(samplerate, sampleformat, channels);
			chan.sampleformat=sampleformat;
			chan.samplerate=samplerate;
			//Calculate the resample factors
			unsigned divider=ggt(output_samplerate*100,samplerate*pitch);
			chan.upsample=(output_samplerate*100)/divider;
			chan.downsample=(samplerate*pitch)/divider;
			
			chan.channels=channels;
			chan.samplesize=AudioDecoder::GetSamplesizeForFormat(sampleformat);
			chan.decoder->SetFade(0,volume,fadein);
			chan.paused=false; //Unpause channel -> Play it.
			Output::Debug("Music: %s, %u, %u, %u\n", file.c_str(),samplerate, chan.upsample,chan.downsample);
			return true;	
		} else {
			fclose(filehandle);
		}
		
		return false;
	}
	
	void LibretroAudio::EnableAudio(bool enabled){
		Muted=!enabled;
	}
	
	void LibretroAudio::AudioThreadCallback(){
		static std::unique_ptr<int16_t> sample_buffer;
		static std::unique_ptr<uint32_t> scrap_buffer;
		static std::unique_ptr<float> mixer_buffer;
		bool channel_active=false;
		float total_volume=0;
		if(RenderAudioFrames==0){
			return;
		}
		if(!sample_buffer){
			sample_buffer.reset(new int16_t[samples_per_frame*output_channels]);
		}
		if(!scrap_buffer){
			scrap_buffer.reset(new uint32_t[samples_per_frame*output_channels]);
		}
		if(!mixer_buffer){
			mixer_buffer.reset(new float[samples_per_frame*output_channels]);
		}
		
		for(unsigned i=0;i< nr_of_bgm_channels;i++){
			if(BGM_Channels[i].decoder && !BGM_Channels[i].paused){
				if(BGM_Channels[i].stopped){
					slock_lock(mutex);
					BGM_Channels[i].decoder.reset();
					slock_unlock(mutex);
				} else {
					
					slock_lock(mutex);
					BGM_Channels[i].decoder->Update(1000/60);	
					float volume=BGM_Channels[i].decoder->GetVolume()/100.0;
					unsigned upsample=BGM_Channels[i].upsample;
					unsigned downsample=BGM_Channels[i].downsample;
					slock_unlock(mutex);
					
					if(volume <= 0){
						//No volume -> no sound ->do nothing
					} else {
						total_volume+=volume;
						BGM_Channels[i].decoder->Decode((uint8_t*)scrap_buffer.get(),(BGM_Channels[i].samplesize*BGM_Channels[i].channels*samples_per_frame*downsample)/upsample);
						if(BGM_Channels[i].decoder->IsFinished()){
							BGM_Channels[i].decoder->Rewind();
							BGM_Channels[i].decoder->SetLooping(true);
							slock_lock(mutex);
							//If another bgm is not just started this moment
							if(!BGM_Channels[i].stopped){
								BGM_PlayedOnceIndicator=true;
							}
							slock_unlock(mutex);
						}
						
						for(unsigned ii=0;ii<samples_per_frame;ii++){
							//Miserable resampler - TODO: Program a good one
							unsigned iii=(ii*downsample)/upsample;
							
							float vall=volume;
							float valr=vall;
							
							
							
							//Convert to floating point
							switch(BGM_Channels[i].sampleformat){
								case AudioDecoder::Format::S8:
									vall*=(((int8_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels]/128.0);
									valr*=(((int8_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels+1]/128.0);
									break;
								case AudioDecoder::Format::U8:
									vall*=(((uint8_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels]/128.0 - 1.0);
									valr*=(((uint8_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels+1]/128.0 - 1.0);
									break;
								case AudioDecoder::Format::S16:
									vall*=(((int16_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels]/32768.0);
									valr*=(((int16_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels+1]/32768.0);
									break;
								case AudioDecoder::Format::U16:
									vall*=(((uint16_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels]/32768.0 - 1.0);
									valr*=(((uint16_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels+1]/32768.0 - 1.0);
									break;
								case AudioDecoder::Format::S32:
									vall*=(((int32_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels]/2147483648.0);
									valr*=(((int32_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels+1]/2147483648.0);
									break;
								case AudioDecoder::Format::U32:
									vall*=(((uint32_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels]/2147483648.0 -1.0);
									valr*=(((uint32_t*)scrap_buffer.get())[iii*BGM_Channels[i].channels+1]/2147483648.0 -1.0);
									break;
								case AudioDecoder::Format::F32:
									vall*=(((float*)scrap_buffer.get())[iii*BGM_Channels[i].channels]);
									valr*=(((float*)scrap_buffer.get())[iii*BGM_Channels[i].channels+1]);
									break;
							}
							
							if(!channel_active){
								//first channel
								mixer_buffer.get()[ii*output_channels]=vall;
								if(BGM_Channels[i].channels>1)mixer_buffer.get()[ii*output_channels+1]=valr;
								else mixer_buffer.get()[ii*output_channels+1]=mixer_buffer.get()[ii*output_channels];
							} else {
								mixer_buffer.get()[ii*output_channels]+=vall;
								if(BGM_Channels[i].channels>1)mixer_buffer.get()[ii*output_channels+1]+=valr;
								else mixer_buffer.get()[ii*output_channels+1]=mixer_buffer.get()[ii*output_channels];								
							}

						}
						channel_active=true;
					}
				}
			}
		}
		if(channel_active){
			
			for(unsigned i=0;i<samples_per_frame*2;i++){
				//TODO: instead of linear scaling implement a dynamic range compression 
				sample_buffer.get()[i]=mixer_buffer.get()[i]*32768.0/total_volume;
			}
			
			RenderAudioFrames(sample_buffer.get(),samples_per_frame);
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
	
	unsigned LibretroAudio::ggt(unsigned a, unsigned b){
		unsigned temp;
		while(b!=0){
			temp=a % b;
			a=b;
			b=temp;
		}
		return a;
	}
	
#endif //USE_LIBRETRO