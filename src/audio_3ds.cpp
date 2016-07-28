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

#if defined(_3DS) && defined(SUPPORT_AUDIO)
#include "audio_3ds.h"
#include "filefinder.h"
#include "output.h"
#include "player.h"

#include <stdio.h>
#include <cstdlib>
#ifdef USE_CACHE
#   include "3ds_cache.h"
#endif
#include "3ds_decoder.h"

// BGM audio streaming thread
volatile bool termStream = false;
DecodedMusic* BGM = NULL;
LightLock BGM_Mutex;
static void streamThread(void* arg){
	
	for(;;) {
		
		// Looks like if we delete this, thread will crash
		svcSleepThread(10000);
		
		// A pretty bad way to close thread
		if(termStream){
			termStream = false;
			threadExit(0);
		}
		
		if (BGM == NULL) continue; // No BGM detected
		else if (BGM->starttick == 0) continue; // BGM not started
		else if (!BGM->isPlaying) continue; // BGM paused
		LightLock_Lock(&BGM_Mutex);
		
		// Calculating delta in milliseconds
		u64 delta = (osGetTime() - BGM->starttick);
		
		// Fade effect feature
		if (BGM->fade_val != 0){
		
			float vol;
			
			// Fade In	
			if (BGM->fade_val > 0){
				vol = (delta * BGM->vol) / float(BGM->fade_val);	
				if (vol >= BGM->vol){
					vol = BGM->vol;
					BGM->fade_val = 0;
				}
			}
			
			// Fade Out	
			else{
				vol = (delta * BGM->vol) / float(-BGM->fade_val);	
				if (vol >= BGM->vol){
					vol = 0.0;
					BGM->fade_val = 0;
				}else vol = BGM->vol - vol;
			}
			
			if (!Player::use_dsp){
				if (BGM->isStereo){
					CSND_SetVol(0x1E, CSND_VOL(vol, -1.0), CSND_VOL(vol, -1.0));
					CSND_SetVol(0x1F, CSND_VOL(vol, 1.0), CSND_VOL(vol, 1.0));
				}else CSND_SetVol(0x1F, CSND_VOL(vol, 0.0), CSND_VOL(vol, 0.0));
				CSND_UpdateInfo(true);
			}else{
				float vol_table[12] = {vol,vol,vol,vol};
				ndspChnSetMix(SOUND_CHANNELS, vol_table);
			}
		}
		
		// Audio streaming feature
		if (BGM->handle != NULL){
			u32 block_mem = BGM->audiobuf_size>>1;
			u32 curPos = BGM->samplerate * BGM->bytepersample * (delta / 1000);
			if (curPos > block_mem * BGM->block_idx) BGM->updateCallback();
		}
		
		LightLock_Unlock(&BGM_Mutex);
		
	}
}

// Audio callbacks
bool csndChnIsPlaying(int ch){
	u8 res;
	csndIsPlaying(ch+0x08, &res);
	return res;
}
void csndClear(int ch){
	CSND_SetPlayState(ch+0x08, 0);
}

// createDspBlock: Create a new block for DSP service
void createDspBlock(ndspWaveBuf* waveBuf, u16 bps, u32 size, bool loop, u32* data){
	waveBuf->data_vaddr = (void*)data;
	waveBuf->nsamples = size / bps;
	waveBuf->looping = loop;
	waveBuf->offset = 0;	
	DSP_FlushDataCache(data, size);
}

CtrAudio::CtrAudio() :
	bgm_volume(0)
{
	if (Player::use_dsp){
		last_ch = 0;
		isPlayingCallback = ndspChnIsPlaying;
		clearCallback = ndspChnWaveBufClear;
		Result res = ndspInit();
		if (res != 0){
			Output::Error("Couldn't initialize audio.\nError code: 0x%X\n", res);
		}
	}else{
		isPlayingCallback = csndChnIsPlaying;
		clearCallback = csndClear;
		Result res = csndInit();
		if (res != 0){
			Output::Error("Couldn't initialize audio.\nError code: 0x%X\n", res);
		}
	}
	
	#ifndef NO_DEBUG
	Output::Debug("Sound initialized successfully!");
	#endif
	
	for (int i=0;i<num_channels;i++){
		audiobuffers[i] = NULL;
	}
	
	#ifndef NO_DEBUG
	Output::Debug("Starting BGM stream thread...");
	#endif
	
	// Starting a secondary thread on SYSCORE for BGM streaming
	LightLock_Init(&BGM_Mutex);
	threadCreate(streamThread, NULL, 32768, 0x18, 1, true);
	
	#ifdef USE_CACHE
	initCache();
	#endif
	
}

CtrAudio::~CtrAudio() {
	
	// Just to be sure to clean up before exiting
	SE_Stop();
	BGM_Stop();
	
	// Closing BGM streaming thread
	termStream = true;
	while (termStream){} // Wait for thread exiting...
	if (BGM != NULL){
		linearFree(BGM->audiobuf);
		BGM->closeCallback();
		free(BGM);
	}
	
	if (Player::use_dsp) ndspExit();
	else csndExit();	
	#ifdef USE_CACHE
	freeCache();
	#endif
}

void CtrAudio::BGM_OnPlayedOnce() {
	// Deprecated
}

void CtrAudio::BGM_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	
	// If a BGM is currently playing, we kill it
	BGM_Stop();
	if (BGM != NULL){
		LightLock_Lock(&BGM_Mutex);
		linearFree(BGM->audiobuf);
		BGM->closeCallback();
		free(BGM);
		BGM = NULL;
		LightLock_Unlock(&BGM_Mutex);
	}

	// Opening and decoding the file
	DecodedMusic* myFile = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	int res = DecodeMusic(file, myFile);
	if (res < 0){
		free(myFile);
		return;
	}else BGM = myFile;
	BGM->starttick = 0;
	
	// Processing music info
	int samplerate = BGM->samplerate;
	BGM->orig_samplerate = BGM->samplerate;
	int codec;
	if (!Player::use_dsp) codec = SOUND_FORMAT(BGM->format);
	else codec = NDSP_CHANNELS(BGM->isStereo + 1) | NDSP_ENCODING(BGM->format);
	
	// Setting music volume
	BGM->vol = volume / 100.0;
	float vol = BGM->vol;
	BGM->fade_val = fadein;
	if (BGM->fade_val != 0){
		vol = 0.0;
	}

	#ifndef NO_DEBUG
	Output::Debug("Playing music %s:",file.c_str());
	Output::Debug("Samplerate: %i",samplerate);
	Output::Debug("Buffer Size: %i bytes",BGM->audiobuf_size);
	#endif
	
	// Starting BGM
	if (!Player::use_dsp){
		if (BGM->isStereo){
			u32 chnbuf_size = BGM->audiobuf_size>>1;
			csndPlaySound(0x1E, SOUND_LINEAR_INTERP | codec | SOUND_REPEAT, samplerate, vol, -1.0, (u32*)BGM->audiobuf, (u32*)BGM->audiobuf, chnbuf_size); // Left
			csndPlaySound(0x1F, SOUND_LINEAR_INTERP | codec | SOUND_REPEAT, samplerate, vol, 1.0, (u32*)(BGM->audiobuf + chnbuf_size), (u32*)(BGM->audiobuf + chnbuf_size), chnbuf_size); // Right		
		}else csndPlaySound(0x1F, SOUND_LINEAR_INTERP | codec | SOUND_REPEAT, samplerate, vol, 0.0, (u32*)BGM->audiobuf, (u32*)BGM->audiobuf, BGM->audiobuf_size);
	}else{
		ndspChnReset(SOUND_CHANNELS);
		ndspChnWaveBufClear(SOUND_CHANNELS);
		ndspChnSetInterp(SOUND_CHANNELS, NDSP_INTERP_LINEAR);
		ndspChnSetRate(SOUND_CHANNELS, float(samplerate));
		ndspChnSetFormat(SOUND_CHANNELS, codec);
		float vol_table[12] = {vol,vol,vol,vol};
		ndspChnSetMix(SOUND_CHANNELS, vol_table);
		createDspBlock(&dspSounds[SOUND_CHANNELS], BGM->bytepersample, BGM->audiobuf_size, true, (u32*)BGM->audiobuf);
		ndspChnWaveBufAdd(SOUND_CHANNELS, &dspSounds[SOUND_CHANNELS]);		
	}
	BGM->isPlaying = true;
	BGM->starttick = osGetTime();
	
}

void CtrAudio::BGM_Pause() {
	if (BGM == NULL) return;
	if (BGM->isPlaying){
		if (!Player::use_dsp){
			CSND_SetPlayState(0x1E, 0);
			CSND_SetPlayState(0x1F, 0);
			CSND_UpdateInfo(true);
		}else ndspChnSetPaused(SOUND_CHANNELS, true);
		BGM->isPlaying = false;
		BGM->starttick = osGetTime()-BGM->starttick; // Save current delta
	}
}

void CtrAudio::BGM_Resume() {
	if (BGM == NULL) return;
	if (!BGM->isPlaying){
		if (!Player::use_dsp){
			if (BGM->isStereo) CSND_SetPlayState(0x1E, 1);
			CSND_SetPlayState(0x1F, 1);
			CSND_UpdateInfo(true);
		}else ndspChnSetPaused(SOUND_CHANNELS, false);
		BGM->isPlaying = true;
		BGM->starttick = osGetTime()-BGM->starttick; // Restore starttick
	}
}

void CtrAudio::BGM_Stop() {
	if (BGM == NULL) return;
	if (!Player::use_dsp){
		CSND_SetPlayState(0x1E, 0);
		CSND_SetPlayState(0x1F, 0);
		CSND_UpdateInfo(true);
	}else ndspChnWaveBufClear(SOUND_CHANNELS);
	BGM->isPlaying = false;
}

bool CtrAudio::BGM_PlayedOnce() {
	if (BGM == NULL) return false;
	return (BGM->block_idx >= BGM->eof_idx);
}

unsigned CtrAudio::BGM_GetTicks() {
	return 0;
}

void CtrAudio::BGM_Volume(int volume) {
	if (BGM == NULL) return;
	float vol = volume / 100.0;
	if (Player::use_dsp){
		if (BGM->isStereo){
			CSND_SetVol(0x1E, CSND_VOL(vol, -1.0), CSND_VOL(vol, -1.0));
			CSND_SetVol(0x1F, CSND_VOL(vol, 1.0), CSND_VOL(vol, 1.0));
		}else CSND_SetVol(0x1F, CSND_VOL(vol, 0.0), CSND_VOL(vol, 0.0));
		CSND_UpdateInfo(true);
	}else{
		float vol_table[12] = {vol,vol,vol,vol};
		ndspChnSetMix(SOUND_CHANNELS, vol_table);
	}
}

void CtrAudio::BGM_Pitch(int pitch) {
	if (BGM == NULL) return;
	LightLock_Lock(&BGM_Mutex);
	
	// Pausing playback to not broke audio streaming
	if (BGM->handle != NULL) BGM_Pause();
	if (!Player::use_dsp) svcSleepThread(100000000); // Temp patch for csnd:SND
	
	// Calculating new samplerate
	u32 new_samplerate = (BGM->orig_samplerate * pitch) / 100;
	if (new_samplerate > 48000) new_samplerate = 48000; // 3DS kinda sucks with samplerates higher then 48000 Hz
	
	// Patching starting tick to not cause issues with audio streaming
	if (BGM->handle != NULL){		
		u64 oldDelta = BGM->starttick;
		u64 newDelta = (BGM->samplerate * oldDelta) / new_samplerate;
		BGM->starttick = newDelta;
	}
	
	// Setting new samplerate
	BGM->samplerate = new_samplerate;
	if (Player::use_dsp) ndspChnSetRate(SOUND_CHANNELS, float(BGM->samplerate));
	else{
		if (BGM->isStereo) CSND_SetTimer(0x1E, CSND_TIMER(BGM->samplerate));
		CSND_SetTimer(0x1F, CSND_TIMER(BGM->samplerate));
		CSND_UpdateInfo(true);
	}
	
	// Resuming playback
	if (BGM->handle != NULL) BGM_Resume();
	
	LightLock_Unlock(&BGM_Mutex);
	
}

void CtrAudio::BGM_Fade(int fade) {
	if (BGM == NULL) return;
	BGM->fade_val = -fade;
}

void CtrAudio::BGS_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	// Deprecated
}

void CtrAudio::BGS_Pause() {
	// Deprecated
}

void CtrAudio::BGS_Resume() {
	// Deprecated
}

void CtrAudio::BGS_Stop() {
	// Deprecated
}

void CtrAudio::BGS_Fade(int fade) {
	// Deprecated
}

int CtrAudio::BGS_GetChannel() const {
	// Deprecated
	return 1;
}

void CtrAudio::ME_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	// Deprecated
}

void CtrAudio::ME_Stop() {
	// Deprecated
}

void CtrAudio::ME_Fade(int fade) {
	// Deprecated
}

void CtrAudio::SE_Play(std::string const& file, int volume, int /* pitch */) {
	
	// Select an available audio channel
	int i = 0;
	if (Player::use_dsp) i = last_ch;
	for(;;){
		if (i >= num_channels){
			if (Player::use_dsp) i = -1;
			else{
				Output::Warning("Cannot execute %s sound: audio-device is busy.\n",file.c_str());
				return;
			}
		}else if (!isPlayingCallback(i)) break;
		i++;
	}
	last_ch = i + 1;
	
	#ifndef USE_CACHE
	if (audiobuffers[i] != NULL){
		linearFree(audiobuffers[i]);
		audiobuffers[i] = NULL;
	}
	#endif
	
	// Init needed vars
	bool isStereo = false;
	int audiobuf_size;
	int codec;
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
	audiobuffers[i] = myFile.audiobuf;
	int samplerate = myFile.samplerate;
	audiobuf_size = myFile.audiobuf_size;
	if (Player::use_dsp) codec = NDSP_CHANNELS(isStereo + 1) | NDSP_ENCODING(myFile.format);
	else codec = SOUND_FORMAT(myFile.format);
	isStereo = myFile.isStereo;
	
	#ifndef NO_DEBUG
	Output::Debug("Playing sound %s:",file.c_str());
	Output::Debug("Samplerate: %i",samplerate);
	Output::Debug("Buffer Size: %i bytes",audiobuf_size);
	Output::Debug("Channel ID: %i",i);
	#endif
	
	// Playing the sound
	float vol = volume / 100.0;
	if (isStereo && (!Player::use_dsp)){
		
		// We need a second channel where to execute right audiochannel since csnd supports only mono sounds natively
		int z = i+1;
		while (z < num_channels){
			if (!isPlayingCallback(z)) break;
			z++;
			if (z >= num_channels){
				Output::Warning("Cannot execute %s sound: audio-device is busy.\n",file.c_str());
				return;
			}
		}
		#ifndef USE_CACHE
		if (audiobuffers[z] != NULL) linearFree(audiobuffers[z]);

		// To not waste CPU clocks, we use a single audiobuffer for both channels so we put just a stubbed audiobuffer on right channel
		audiobuffers[z] = (u8*)linearAlloc(1);
		#endif
		
		int chnbuf_size = audiobuf_size>>1;
		csndPlaySound(i+0x08, SOUND_LINEAR_INTERP | codec, samplerate, vol, -1.0, (u32*)audiobuffers[i], (u32*)audiobuffers[i], chnbuf_size); // Left
		csndPlaySound(z+0x08, SOUND_LINEAR_INTERP | codec, samplerate, vol, 1.0, (u32*)(audiobuffers[i] + chnbuf_size), (u32*)(audiobuffers[i] + chnbuf_size), chnbuf_size); // Right
		
	}else{
		if (Player::use_dsp){
			ndspChnReset(i);
			ndspChnWaveBufClear(i);
			ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
			ndspChnSetRate(i, float(samplerate));
			ndspChnSetFormat(i, codec);
			float vol_table[12] = {vol,vol,vol,vol};
			ndspChnSetMix(i, vol_table);
			createDspBlock(&dspSounds[i], myFile.bytepersample, audiobuf_size, false, (u32*)audiobuffers[i]);
			ndspChnWaveBufAdd(i, &dspSounds[i]);
		}else csndPlaySound(i+0x08, SOUND_LINEAR_INTERP | codec, samplerate, vol, 0.0, (u32*)audiobuffers[i], (u32*)audiobuffers[i], audiobuf_size);
	}
}

void CtrAudio::SE_Stop() {
	for(int i=0;i<num_channels;i++){
		clearCallback(i);
		#ifndef USE_CACHE
		if (audiobuffers[i] != NULL) linearFree(audiobuffers[i]);
		audiobuffers[i] = NULL;
		#endif
		if (Player::use_dsp) ndspChnWaveBufClear(i);	}
	if (!Player::use_dsp) CSND_UpdateInfo(true);}

void CtrAudio::Update() {	
	
	#ifndef USE_CACHE
	// Closing and freeing finished sounds	
	for(int i=0;i<num_channels;i++){
		if (audiobuffers[i] != NULL){
			if (!isPlayingCallback(i)){
				linearFree(audiobuffers[i]);
				audiobuffers[i] = NULL;
				if (Player::use_dsp) ndspChnWaveBufClear(i);
			}
		}
	}
	#endif
	
}

#endif
