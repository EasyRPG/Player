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
 
#include "system.h"
#include "audio_3ds.h"
#include "filefinder.h"
#include "output.h"

#ifdef _3DS
#include <stdio.h>
#ifdef USE_CACHE
#include "3ds_cache.h"
#else
#include "3ds_decoder.h"
#endif
CtrAudio::CtrAudio() :
	bgm_volume(0),
	bgs_channel(0),
	bgs_playing(false),
	me_channel(0),
	me_stopped_bgm(false)
{
	Result res = csndInit();
	if (res != 0){
		Output::Error("Couldn't initialize audio.\nError code: 0x%X\n", res);
	}
	
	#ifndef NO_DEBUG
	Output::Debug("Sound initialized successfully!\n");
	#endif
	
	for (int i=0;i<num_channels;i++){
		audiobuffers[i] = NULL;
	}
	
	#ifdef USE_CACHE
	initCache();
	#endif
	
}

CtrAudio::~CtrAudio() {
	SE_Stop(); // Just to be sure to clean up before exiting
	csndExit();	
	#ifdef USE_CACHE
	freeCache();
	#endif
}

void CtrAudio::BGM_OnPlayedOnce() {

}

void CtrAudio::BGM_Play(std::string const& file, int volume, int /* pitch */, int fadein) {

}

void CtrAudio::BGM_Pause() {

}

void CtrAudio::BGM_Resume() {

}

void CtrAudio::BGM_Stop() {

}

bool CtrAudio::BGM_PlayedOnce() {
	return true;
}

unsigned CtrAudio::BGM_GetTicks() {
	return 0;
}

void CtrAudio::BGM_Volume(int volume) {

}

void CtrAudio::BGM_Pitch(int /* pitch */) {

}

void CtrAudio::BGM_Fade(int fade) {

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
	while (i < num_channels){
		u8 isPlaying;
		csndIsPlaying(i+0x08, &isPlaying);
		if (!isPlaying) break;
		i++;
		if (i >= num_channels){
			Output::Warning("Cannot execute %s sound: audio-device is busy.\n",file.c_str());
			return;
		}
	}
	if (audiobuffers[i] != NULL){
		#ifndef USE_CACHE
		linearFree(audiobuffers[i]);
		#endif
		audiobuffers[i] = NULL;
	}
	
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
	
		// Searching for the file
		std::string const path = FileFinder::FindSound(file);
		if (path.empty()) {
			Output::Debug("Sound not found: %s", file.c_str());
			return;
		}
	
		// Opening and decoding the file (TODO: Add other containers support like OGG and MIDI files)
		int res = DecodeWav(path, &myFile);
		if (res < 0) return;
		#ifdef USE_CACHE
		else sprintf(soundtable[res],"%s",file.c_str());
		#endif
		
	#ifdef USE_CACHE
	}else myFile = decodedtable[cacheIdx];
	#endif
	
	// Processing sounds info
	audiobuffers[i] = myFile.audiobuf;
	samplerate = myFile.samplerate;
	audiobuf_size = myFile.audiobuf_size;
	codec = SOUND_FORMAT(myFile.format);
	isStereo = myFile.isStereo;
	
	#ifndef NO_DEBUG
	Output::Debug("Playing sound %s:",file.c_str());
	Output::Debug("Samplerate: %i",samplerate);
	Output::Debug("Buffer Size: %i bytes",audiobuf_size);
	#endif
	
	// Playing the sound
	float vol = volume / 100.0;
	if (isStereo){
		
		// We need a second channel where to execute right audiochannel since csnd supports only mono sounds natively
		int z = i+1;
		while (z < num_channels){
			u8 isPlaying;
			csndIsPlaying(z+0x08, &isPlaying);
			if (!isPlaying) break;
			z++;
			if (z >= num_channels){
				Output::Warning("Cannot execute %s sound: audio-device is busy.\n",file.c_str());
				return;
			}
		}
		#ifndef USE_CACHE
		if (audiobuffers[z] != NULL) linearFree(audiobuffers[z]);
		#endif
		
		// To not waste CPU clocks, we use a single audiobuffer for both channels so we put just a stubbed audiobuffer on right channel
		#ifndef USE_CACHE
		audiobuffers[z] = audiobuffers[i]; // If we use cache we only need to be sure the audiobuffer is not NULL
		#else
		audiobuffers[z] = (u8*)linearAlloc(1);
		#endif
		int chnbuf_size = audiobuf_size>>1;
		csndPlaySound(i+0x08, SOUND_LINEAR_INTERP | codec, samplerate, vol, -1.0, (u32*)audiobuffers[i], (u32*)audiobuffers[i], chnbuf_size); // Left
		csndPlaySound(z+0x08, SOUND_LINEAR_INTERP | codec, samplerate, vol, 1.0, (u32*)(audiobuffers[i] + chnbuf_size), ((u32*)audiobuffers[i] + chnbuf_size), chnbuf_size); // Right
		
	}else csndPlaySound(i+0x08, SOUND_LINEAR_INTERP | codec, samplerate, vol, 0.0, (u32*)audiobuffers[i], (u32*)audiobuffers[i], audiobuf_size);
	
}

void CtrAudio::SE_Stop() {
	for(int i=0;i<num_channels;i++){
		CSND_SetPlayState(i+0x08, 0);
		#ifndef USE_CACHE
		if (audiobuffers[i] != NULL) linearFree(audiobuffers[i]);
		#endif
		audiobuffers[i] = NULL;
	}
	CSND_UpdateInfo(0);
}

void CtrAudio::Update() {
	for(int i=0;i<num_channels;i++){
		if (audiobuffers[i] != NULL){
			u8 isPlaying;
			csndIsPlaying(i+0x08, &isPlaying);
			if (!isPlaying){
				#ifndef USE_CACHE
				linearFree(audiobuffers[i]);
				#endif
				audiobuffers[i] = NULL;
			}
		}
	}
}

#endif