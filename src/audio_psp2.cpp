/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(PSP2) && defined(SUPPORT_AUDIO)
#include "audio_psp2.h"
#include "filefinder.h"
#include "output.h"
#include "player.h"
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <cstdlib>
#include "audio_decoder.h"
#include <atomic>

// Internal stuffs
#define AUDIO_CHANNELS 7 // PSVITA has 8 audio channels but one is used for BGM
#define SFX_QUEUE_SIZE 8
#define AUDIO_BUFSIZE 8192 // Max dimension of BGM/SFX buffer size
SceUID sfx_threads[AUDIO_CHANNELS];
uint16_t bgm_chn = 0xDEAD;
std::unique_ptr<AudioDecoder> audio_decoder;
std::unique_ptr<AudioDecoder> sfx_decoder[8];
uint8_t cur_decoder = 0;

// Sound block struct
struct DecodedSound{
	uint8_t* audiobuf;
	uint8_t* audiobuf2;
	uint8_t* cur_audiobuf;
	FILE* handle;
	bool endedOnce;
	bool isPlaying;
	bool isStereo;
	int pitch;
	int vol;
	uint8_t id;
};

// Music block struct
struct DecodedMusic{
	bool isStereo;
	uint8_t* audiobuf;
	uint8_t* audiobuf2;
	uint8_t* cur_audiobuf;
	FILE* handle;
	bool isNewTrack;
	int tick;
	bool endedOnce;
	bool isPlaying;
	int vol;
};

// BGM audio streaming thread
volatile bool termStream = false;
DecodedMusic* BGM = NULL;
SceUID BGM_Mutex;
SceUID BGM_Thread;
static int streamThread(unsigned int args, void* arg){
	
	int vol, dec_vol;
	
	for(;;) {
		
		// A pretty bad way to close thread
		if(termStream){
			termStream = false;
			if (bgm_chn != 0xDEAD){
				sceAudioOutReleasePort(bgm_chn);
				bgm_chn = 0xDEAD;
			}
			sceKernelExitThread(0);
		}
		
		sceKernelWaitSema(BGM_Mutex, 1, NULL);
		if (BGM == NULL || (!BGM->isPlaying)){
			sceKernelSignalSema(BGM_Mutex, 1);
			continue;
		}
		
		// Seems like audio ports are thread dependant on PSVITA :/
		if (BGM->isNewTrack){
			uint8_t audio_mode = BGM->isStereo ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO;
			int nsamples = AUDIO_BUFSIZE / ((audio_mode+1)<<1);
			if (bgm_chn == 0xDEAD) bgm_chn = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, nsamples, 48000, audio_mode);
			sceAudioOutSetConfig(bgm_chn, nsamples, 48000, audio_mode);
			vol = BGM->vol * 327;
			int vol_stereo[] = {vol, vol};
			sceAudioOutSetVolume(bgm_chn, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol_stereo);	
			BGM->isNewTrack = false;
		}
		
		// Volume changes support
		dec_vol = audio_decoder->GetVolume();
		if (dec_vol != vol){
			vol = dec_vol * 327;
			int vol_stereo[] = {vol, vol};
			sceAudioOutSetVolume(bgm_chn, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol_stereo);
			vol = dec_vol;
		}
		
		
		// Audio streaming feature
		if (BGM->handle != NULL){
			if (BGM->cur_audiobuf == BGM->audiobuf) BGM->cur_audiobuf = BGM->audiobuf2;
			else BGM->cur_audiobuf = BGM->audiobuf;
			audio_decoder->Decode(BGM->cur_audiobuf, AUDIO_BUFSIZE);	
			if (audio_decoder->GetLoopCount() > 0){ // EoF
				BGM->endedOnce = true;
			}
			int res = sceAudioOutOutput(bgm_chn, BGM->cur_audiobuf);
			if (res < 0) Output::Error("An error occurred in audio thread (0x%lX)", res);
		}
		
		sceKernelSignalSema(BGM_Mutex, 1);
		
	}
}

// SFX audio thread
SceUID SFX_Mutex;
volatile bool mustExit = false;
DecodedSound* sfx_sounds[SFX_QUEUE_SIZE];
uint8_t output_idx = 0;
uint8_t input_idx = 0;
uint8_t sfx_exited = 0;
std::atomic<std::uint8_t> availThreads(AUDIO_CHANNELS);
static int sfxThread(unsigned int args, void* arg){
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, 64, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	if (ch < 0){
		Output::Warning("SFX Thread: Cannot open audio port");
		sceKernelExitThread(0);
	}
	for (;;){
		sceKernelWaitSema(SFX_Mutex, 1, NULL);
		
		// Check if the thread must be closed
		if (mustExit){
			sfx_exited++;
			if (sfx_exited < AUDIO_CHANNELS) sceKernelSignalSema(SFX_Mutex, 1);
			else mustExit = false;
			sceAudioOutReleasePort(ch);
			sceKernelExitThread(0);
		}
		
		DecodedSound* sfx = sfx_sounds[output_idx++];
		if (output_idx >= SFX_QUEUE_SIZE) output_idx = 0;
		
		// Preparing audio port
		uint8_t audio_mode = sfx->isStereo ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO;
		int nsamples = AUDIO_BUFSIZE / ((audio_mode+1)<<1);
		sceAudioOutSetConfig(ch, nsamples, 48000, audio_mode);
		int vol = sfx->vol * 327;
		int vol_stereo[] = {vol, vol};
		sceAudioOutSetVolume(ch, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol_stereo);	
		
		// Applying pitch
		sfx_decoder[sfx->id]->SetPitch(sfx->pitch);
		
		// Playing sound
		while (!sfx->endedOnce){
			if (sfx->cur_audiobuf == sfx->audiobuf) sfx->cur_audiobuf = sfx->audiobuf2;
			else sfx->cur_audiobuf = sfx->audiobuf;
			sfx_decoder[sfx->id]->Decode(sfx->cur_audiobuf, AUDIO_BUFSIZE);	
			if (sfx_decoder[sfx->id]->IsFinished()){ // EoF
				sfx->endedOnce = true;
			}
			sceAudioOutOutput(ch, sfx->cur_audiobuf);
		}
		
		// Freeing sound
		free(sfx->audiobuf);
		free(sfx->audiobuf2);
		sfx_decoder[sfx->id].reset();
		free(sfx);
		availThreads++;
		
	}
	
}

Psp2Audio::Psp2Audio() :
	bgm_volume(0)
{
	
	// Creating mutexs
	BGM_Mutex = sceKernelCreateSema("BGM Mutex", 0, 1, 1, NULL);
	SFX_Mutex = sceKernelCreateSema("SFX Mutex", 0, 0, 1, NULL);
	
	// Starting audio thread for BGM
	BGM_Thread = sceKernelCreateThread("BGM Thread", &streamThread, 0x10000100, 0x10000, 0, 0, NULL);
	int res = sceKernelStartThread(BGM_Thread, sizeof(BGM_Thread), &BGM_Thread);
	if (res != 0){
		Output::Error("Failed to init audio thread (0x%x)", res);
		return;
	}
	
	// Starting audio threads for SFX
	for (int i=0;i < AUDIO_CHANNELS; i++){
		sfx_threads[i] = sceKernelCreateThread("SFX Thread", &sfxThread, 0x10000100, 0x10000, 0, 0, NULL);
		int res = sceKernelStartThread(sfx_threads[i], sizeof(sfx_threads[i]), &sfx_threads[i]);
		if (res != 0){
			Output::Error("Failed to init audio thread (0x%x)", res);
			return;
		}
	}
	
}

Psp2Audio::~Psp2Audio() {
	
	// Just to be sure to clean up before exiting
	SE_Stop();
	BGM_Stop();
	
	// Closing BGM streaming thread
	termStream = true;
	while (termStream){} // Wait for thread exiting...
	sceKernelDeleteThread(BGM_Thread);
	if (BGM != NULL){
		free(BGM->audiobuf);
		audio_decoder.reset();
		free(BGM);
	}
	
	// Starting exit procedure for sfx threads
	mustExit = true;
	sceKernelSignalSema(SFX_Mutex, 1);
	while (mustExit){} // Wait for threads exiting...
	sfx_exited = 0;
	
	// Deleting mutexs and sfx threads
	sceKernelDeleteSema(BGM_Mutex);
	for (int i=0;i<AUDIO_CHANNELS;i++){
		sceKernelDeleteThread(sfx_threads[i]);
	}
	sceKernelDeleteSema(SFX_Mutex);
	
}

void Psp2Audio::BGM_Play(std::string const& file, int volume, int pitch, int fadein) {
	
	// If a BGM is currently playing, we kill it
	BGM_Stop();
	sceKernelWaitSema(BGM_Mutex, 1, NULL);
	if (BGM != NULL){
		free(BGM->audiobuf);
		free(BGM->audiobuf2);
		audio_decoder.reset();
		free(BGM);
		BGM = NULL;
	}
	
	// Opening file
	FILE* stream = FileFinder::fopenUTF8(file, "rb");
	if (!stream) {
		Output::Warning("Couldn't open music file %s", file.c_str());
		sceKernelSignalSema(BGM_Mutex, 1);
		return;
	}
	
	// Trying to use internal decoder
	audio_decoder = AudioDecoder::Create(stream, file);
	if (audio_decoder == NULL){
		fclose(stream);
		Output::Warning("Unsupported music format (%s)", file.c_str());
		sceKernelSignalSema(BGM_Mutex, 1);
		return;
	}
	
	// Initializing internal audio decoder
	int audiotype;
	fseek(stream, 0, SEEK_SET);
	if (!audio_decoder->Open(stream)) Output::Error("An error occured in audio decoder (%s)", audio_decoder->GetError().c_str());
	audio_decoder->SetLooping(true);
	AudioDecoder::Format int_format;
	int samplerate;	
	audio_decoder->SetFormat(48000, AudioDecoder::Format::S16, 2);
	audio_decoder->GetFormat(samplerate, int_format, audiotype);
	if (samplerate != 48000) Output::Warning("Cannot resample music file. Music will be distorted.");
	
	// Initializing music block
	DecodedMusic* myFile = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	
	// Check for file audiocodec
	if (audiotype == 2) myFile->isStereo = true;
	else myFile->isStereo = false;
	
	// Setting audiobuffer size
	myFile->audiobuf = (uint8_t*)malloc(AUDIO_BUFSIZE);
	myFile->audiobuf2 = (uint8_t*)malloc(AUDIO_BUFSIZE);
	myFile->cur_audiobuf = myFile->audiobuf;	
	
	//Setting default streaming values
	myFile->handle = stream;
	myFile->endedOnce = false;
	
	// Passing new music block to the audio thread
	BGM = myFile;
	
	// Music settings
	audio_decoder->SetFade(0, volume, fadein);
	audio_decoder->SetPitch(pitch);
	BGM->tick = DisplayUi->GetTicks();
	BGM->vol = volume;
	
	// Starting BGM
	BGM->isNewTrack = true;
	BGM->isPlaying = true;
	sceKernelSignalSema(BGM_Mutex, 1);
	
}

void Psp2Audio::BGM_Pause() {
	sceKernelWaitSema(BGM_Mutex, 1, NULL);
	if (BGM != NULL && BGM->isPlaying) BGM->isPlaying = false;
	sceKernelSignalSema(BGM_Mutex, 1);
}

void Psp2Audio::BGM_Resume() {
	sceKernelWaitSema(BGM_Mutex, 1, NULL);
	if (BGM != NULL && (!BGM->isPlaying)) BGM->isPlaying = true;
	sceKernelSignalSema(BGM_Mutex, 1);
}

void Psp2Audio::BGM_Stop() {
	sceKernelWaitSema(BGM_Mutex, 1, NULL);
	if (BGM != NULL) BGM->isPlaying = false;
	sceKernelSignalSema(BGM_Mutex, 1);
}

bool Psp2Audio::BGM_PlayedOnce() const {
	return (BGM == NULL) ? false : BGM->endedOnce;
}

bool Psp2Audio::BGM_IsPlaying() const {
	return BGM->isPlaying;
}

unsigned Psp2Audio::BGM_GetTicks() const {
	if (BGM != NULL) return audio_decoder->GetTicks();
	else return 0;
}

void Psp2Audio::BGM_Volume(int volume) {
	audio_decoder->SetVolume(volume);
}

void Psp2Audio::BGM_Pitch(int pitch) {
	audio_decoder->SetPitch(pitch);	
}

void Psp2Audio::BGM_Fade(int fade) {
	if (BGM == NULL) return;
	audio_decoder->SetFade(audio_decoder->GetVolume(), 0, fade);
	BGM->tick = DisplayUi->GetTicks();
}

void Psp2Audio::SE_Play(std::string const& file, int volume, int pitch) {
	
	// Allocating DecodedSound object
	DecodedSound* myFile = (DecodedSound*)malloc(sizeof(DecodedSound));
	
	// Opening file
	FILE* stream = FileFinder::fopenUTF8(file, "rb");
	if (!stream) {
		Output::Warning("Couldn't open sound file %s", file.c_str());
		return;
	}
	
	// Check if we have at least an available audio thread
	if (availThreads > 0){
		
		// Trying to use internal decoder
		availThreads--;
		uint8_t id = cur_decoder++;
		if (cur_decoder > 7) cur_decoder = 0;
		sfx_decoder[id] = AudioDecoder::Create(stream, file);
		if (sfx_decoder[id] == NULL){
			fclose(stream);
			Output::Warning("Unsupported sound format (%s)", file.c_str());
			return;
		}

		// Initializing internal audio decoder
		int audiotype;
		fseek(stream, 0, SEEK_SET);
		if (!sfx_decoder[id]->Open(stream)) Output::Error("An error occured in audio decoder (%s)", audio_decoder->GetError().c_str());
		sfx_decoder[id]->SetLooping(false);
		AudioDecoder::Format int_format;
		int samplerate;	
		sfx_decoder[id]->SetFormat(48000, AudioDecoder::Format::S16, 2);
		sfx_decoder[id]->GetFormat(samplerate, int_format, audiotype);
		if (samplerate != 48000) Output::Warning("Cannot resample sound file. Sound will be distorted.");
		myFile->id = id;
	
		// Check for file audiocodec
		if (audiotype == 2) myFile->isStereo = true;
		else myFile->isStereo = false;
	
		// Setting audiobuffer size
		myFile->audiobuf = (uint8_t*)malloc(AUDIO_BUFSIZE);
		myFile->audiobuf2 = (uint8_t*)malloc(AUDIO_BUFSIZE);
		myFile->cur_audiobuf = myFile->audiobuf;	
	
		//Setting default streaming values
		myFile->handle = stream;
		myFile->endedOnce = false;
	
		// Passing pitch and volume values to the object
		myFile->pitch = pitch;
		myFile->vol = volume;
	
		// Passing sound to an sfx thread
		sfx_sounds[input_idx++] = myFile;
		if (input_idx >= SFX_QUEUE_SIZE) input_idx = 0;
		sceKernelSignalSema(SFX_Mutex, 1);
	
	}else{
		fclose(stream);
		Output::Warning("Cannot reproduce audio sound. No channels available.");
	}
}

void Psp2Audio::SE_Stop() {
	
}

void Psp2Audio::Update() {	
	if (BGM != NULL && BGM->tick > 0){
		int t = DisplayUi->GetTicks();
		audio_decoder->Update(t - BGM->tick);
		BGM->tick = t;
	}
}

#endif
