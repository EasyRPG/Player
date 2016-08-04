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

uint16_t bgm_chn = 0xDEAD;

// osGetTime implementation
uint64_t osGetTime(void){
	return (sceKernelGetProcessTimeWide() / 1000);
}

// BGM audio streaming thread
volatile bool termStream = false;
DecodedMusic* BGM = NULL;
SceUID BGM_Mutex;
static int streamThread(unsigned int args, void* arg){
	
	for(;;) {
		
		// A pretty bad way to close thread
		if(termStream){
			termStream = false;
			sceKernelExitThread(0);
		}
		
		sceKernelWaitSema(BGM_Mutex, 1, NULL);
		if (BGM == NULL || BGM->starttick == 0 || (!BGM->isPlaying)){
			sceKernelSignalSema(BGM_Mutex, 1);
			continue;
		}
		
		// Seems like audio ports are thread dependant on PSVITA :/
		if (bgm_chn == 0xDEAD){
			bgm_chn = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, BGM_BUFSIZE, BGM->orig_samplerate, SCE_AUDIO_OUT_MODE_STEREO);
			if (bgm_chn < 0) Output::Error("Cannot open BGM audio port. (0x%lX)", bgm_chn);
			sceAudioOutSetConfig(bgm_chn, -1, -1, -1);
			sceAudioOutSetVolume(bgm_chn, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, &BGM->vol);	
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

Psp2Audio::Psp2Audio() :
	bgm_volume(0)
{
	
	// Creating a mutex
	BGM_Mutex = sceKernelCreateSema("BGM Mutex", 0, 1, 1, NULL);
	
	// Starting audio thread for BGM
	SceUID audiothread = sceKernelCreateThread("Audio Thread", &streamThread, 0x10000100, 0x10000, 0, 0, NULL);
	int res = sceKernelStartThread(audiothread, sizeof(audiothread), &audiothread);
	if (res != 0){
		Output::Error("Failed to init audio thread (0x%x)", res);
		return;
	}
	
	#ifdef USE_CACHE
	initCache();
	#endif
	
}

Psp2Audio::~Psp2Audio() {
	
	// Just to be sure to clean up before exiting
	SE_Stop();
	BGM_Stop();
	
	// Closing BGM streaming thread
	termStream = true;
	while (termStream){} // Wait for thread exiting...
	if (BGM != NULL){
		free(BGM->audiobuf);
		BGM->closeCallback();
		free(BGM);
	}
	
	#ifdef USE_CACHE
	freeCache();
	#endif
	
	// Deleting mutex
	sceKernelDeleteSema(BGM_Mutex);
	
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
	BGM->starttick = 0;
	
	// Processing music info
	int samplerate = BGM->samplerate;
	BGM->orig_samplerate = BGM->samplerate;
	
	// Setting music volume
	BGM->vol = volume * 327;
	int vol = BGM->vol;
	BGM->fade_val = fadein;
	if (BGM->fade_val != 0){
		vol = 0;
	}

	#ifndef NO_DEBUG
	Output::Debug("Playing music %s:",file.c_str());
	Output::Debug("Samplerate: %i",samplerate);
	Output::Debug("Buffer Size: %i bytes",BGM->audiobuf_size);
	#endif
	
	// Starting BGM
	BGM->isPlaying = true;
	BGM->starttick = osGetTime();
	sceKernelSignalSema(BGM_Mutex, 1);
	
}

void Psp2Audio::BGM_Pause() {
	sceKernelWaitSema(BGM_Mutex, 1, NULL);
	if (BGM != NULL && BGM->isPlaying){
		BGM->isPlaying = false;
		BGM->starttick = osGetTime()-BGM->starttick; // Save current delta
	}
	sceKernelSignalSema(BGM_Mutex, 1);
}

void Psp2Audio::BGM_Resume() {
	sceKernelWaitSema(BGM_Mutex, 1, NULL);
	if (BGM != NULL && (!BGM->isPlaying)){
		BGM->isPlaying = true;
		BGM->starttick = osGetTime()-BGM->starttick; // Restore starttick
	}
	sceKernelSignalSema(BGM_Mutex, 1);
}

void Psp2Audio::BGM_Stop() {
	sceKernelWaitSema(BGM_Mutex, 1, NULL);
	if (BGM != NULL) BGM->isPlaying = false;
	sceKernelSignalSema(BGM_Mutex, 1);
}

bool Psp2Audio::BGM_PlayedOnce() const {
	if (BGM == NULL) return false;
	return (BGM->block_idx >= BGM->eof_idx);
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
	
	// Init needed vars
	bool isStereo = false;
	int audiobuf_size;
	DecodedSound myFile;
	
	#ifdef USE_CACHE
	// Looking if the sound is in sounds cache
	int cacheIdx = lookCache(file.c_str());
	if (cacheIdx < 0){
	#endif
	
		// Opening and decoding the file
		int res = DecodeSound(file, &myFile);
		if (res < 0) return;
		#ifdef USE_CACHE
		else sprintf(soundtable[res],"%s",file.c_str());
		#endif
		
	#ifdef USE_CACHE
	}else myFile = decodedtable[cacheIdx];
	#endif
	
	// Processing sound info
	uint8_t* audiobuf = myFile.audiobuf;
	int samplerate = myFile.samplerate;
	audiobuf_size = myFile.audiobuf_size;
	
	isStereo = myFile.isStereo;
	
	#ifndef NO_DEBUG
	Output::Debug("Playing sound %s:",file.c_str());
	Output::Debug("Samplerate: %i",samplerate);
	Output::Debug("Buffer Size: %i bytes",audiobuf_size);
	#endif
	
	// Playing the sound
	int cur_pos = 0;
	int vol = volume * 327;
	int buf_size = SCE_AUDIO_MAX_LEN;
	if (audiobuf_size < SCE_AUDIO_MAX_LEN) buf_size = audiobuf_size;
	int chn = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_VOICE, buf_size, samplerate, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(chn, -1, -1, -1);
	sceAudioOutSetVolume(chn, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, &vol);
	while (cur_pos < audiobuf_size){
		sceAudioOutOutput(chn, &audiobuf[cur_pos]);
		cur_pos += buf_size;
	}
	sceAudioOutReleasePort(chn);
}

void Psp2Audio::SE_Stop() {
	
}

void Psp2Audio::Update() {	
	
}

#endif
