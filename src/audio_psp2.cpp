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
#define AUDIO_CHANNELS 7
SceUID sfx_threads[AUDIO_CHANNELS];
uint16_t bgm_chn = 0xDEAD;

// osGetTime implementation
uint64_t osGetTime(void){
	return (sceKernelGetProcessTimeWide() / 1000);
}

// BGM audio streaming thread
volatile bool termStream = false;
DecodedMusic* BGM = NULL;
SceUID BGM_Mutex;
SceUID BGM_Thread;
static int streamThread(unsigned int args, void* arg){
	
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
			if (bgm_chn == 0xDEAD) bgm_chn = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, nsamples, BGM->orig_samplerate, audio_mode);
			sceAudioOutSetConfig(bgm_chn, nsamples, BGM->orig_samplerate, audio_mode);
			sceAudioOutSetVolume(bgm_chn, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, &BGM->vol);	
			BGM->isNewTrack = false;
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
SceUID SFX_Mutex[AUDIO_CHANNELS];
DecodedSound* sfx_sounds[4];
uint8_t thread_idx = 0;
uint8_t output_idx = 0;
uint8_t input_idx = 0;
static int sfxThread(unsigned int args, void* arg){
	uint8_t id = thread_idx++;
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, 64, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	if (ch < 0){
		Output::Warning("SFX Thread: Cannot open audio port");
		sceKernelExitThread(0);
	}
	for (;;){
		sceKernelWaitSema(SFX_Mutex[id], 1, NULL);
		DecodedSound* sfx = sfx_sounds[output_idx++];
		if (output_idx > 3) output_idx = 0;
		
		// Preparing audio port
		uint8_t audio_mode = sfx->isStereo ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO;
		int nsamples = BGM_BUFSIZE / ((audio_mode+1)<<1);
		sceAudioOutSetConfig(ch, nsamples, sfx->samplerate, audio_mode);
		sceAudioOutSetVolume(ch, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, &sfx->vol);	
		
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
	for (int i=0;i<AUDIO_CHANNELS;i++) SFX_Mutex[i] = sceKernelCreateSema("SFX Mutex", 0, 0, 1, NULL);
	
	// Starting audio thread for BGM
	BGM_Thread = sceKernelCreateThread("BGM Thread", &streamThread, 0x10000100, 0x10000, 0, 0, NULL);
	int res = sceKernelStartThread(BGM_Thread, sizeof(BGM_Thread), &BGM_Thread);
	if (res != 0){
		Output::Error("Failed to init audio thread (0x%x)", res);
		return;
	}
	
	// Starting audio threads for SFX
	for (int i=0;i < AUDIO_CHANNELS; i++){
		sfx_threads[i] = sceKernelCreateThread("BGM Thread", &sfxThread, 0x10000100, 0x10000, 0, 0, NULL);
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
	
	// Deleting mutexs and sfx threads
	sceKernelDeleteSema(BGM_Mutex);
	for (int i=0;i<AUDIO_CHANNELS;i++){
		sceKernelDeleteThread(sfx_threads[i]);
		sceKernelDeleteSema(SFX_Mutex[i]);
	}
	
}

void Psp2Audio::BGM_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	
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
	
	// Processing music info
	int samplerate = BGM->samplerate;
	BGM->orig_samplerate = BGM->samplerate;
	
	// Setting music volume
	BGM->vol = volume * 327;

	#ifndef NO_DEBUG
	Output::Debug("Playing music %s:",file.c_str());
	Output::Debug("Samplerate: %i",samplerate);
	Output::Debug("Buffer Size: %i bytes",BGM->audiobuf_size);
	#endif
	
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
	return 0;
}

void Psp2Audio::BGM_Volume(int volume) {
	if (BGM == NULL) return;
}

void Psp2Audio::BGM_Pitch(int pitch) {
	if (BGM == NULL) return;	
}

void Psp2Audio::BGM_Fade(int fade) {
	if (BGM == NULL) return;
	BGM->fade_val = -fade;
}

void Psp2Audio::SE_Play(std::string const& file, int volume, int /* pitch */) {

	if (thread_idx >= AUDIO_CHANNELS) thread_idx = 0;
	
	// Allocating DecodedSound object
	DecodedSound* myFile = (DecodedSound*)malloc(sizeof(DecodedSound));
	
	// Opening the file
	int res = DecodeSound(file, myFile);
	if (res < 0) return;
	
	// Passing sound to an sfx thread
	sfx_sounds[input_idx++] = myFile;
	if (input_idx > 3) input_idx = 0;
	sceKernelSignalSema(SFX_Mutex[thread_idx++], 1);
	
}

void Psp2Audio::SE_Stop() {
	
}

void Psp2Audio::Update() {	
	
}

#endif
