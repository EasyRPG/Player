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
#ifdef USE_CACHE
#   include "3ds_cache.h"
#endif
#include "psp2_decoder.h"

// Internal stuffs
#define AUDIO_CHANNELS 7 // PSVITA has 8 audio channels but one is used for BGM
#define SFX_QUEUE_SIZE 8
SceUID sfx_threads[AUDIO_CHANNELS];
uint16_t bgm_chn = 0xDEAD;
extern std::unique_ptr<AudioDecoder> audio_decoder;

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
			int nsamples = BGM_BUFSIZE / ((audio_mode+1)<<1);
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
			BGM->updateCallback();
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
		int nsamples = BGM_BUFSIZE / ((audio_mode+1)<<1);
		sceAudioOutSetConfig(ch, nsamples, sfx->samplerate, audio_mode);
		int vol = sfx->vol * 327;
		int vol_stereo[] = {vol, vol};
		sceAudioOutSetVolume(ch, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol_stereo);	
		
		// Applying pitch
		sfx->pitchCallback(sfx);
		
		// Playing sound
		while (!sfx->endedOnce){
			sfx->updateCallback(sfx);
			sceAudioOutOutput(ch, sfx->cur_audiobuf);
		}
		
		// Freeing sound
		free(sfx->audiobuf);
		free(sfx->audiobuf2);
		sfx->closeCallback(sfx);
		free(sfx);
		
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
		BGM->closeCallback();
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
		BGM->closeCallback();
		free(BGM);
		BGM = NULL;
	}
	
	// Opening and decoding the file
	DecodedMusic* myFile = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	int res = DecodeMusic(file, myFile);
	if (res < 0){
		free(myFile);
		sceKernelSignalSema(BGM_Mutex, 1);
		return;
	}else BGM = myFile;
	
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
	
	// Opening the file
	int res = DecodeSound(file, myFile);
	if (res < 0) return;
	
	// Passing pitch and volume values to the object
	myFile->pitch = pitch;
	myFile->vol = volume;
	
	// Passing sound to an sfx thread
	sfx_sounds[input_idx++] = myFile;
	if (input_idx >= SFX_QUEUE_SIZE) input_idx = 0;
	sceKernelSignalSema(SFX_Mutex, 1);
	
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
