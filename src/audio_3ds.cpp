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
#include "output.h"

#ifdef _3DS
#include <3ds.h>
#include <stdio.h>

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
		printf("Sound initialized successfully!\n");
	#endif
	
	int const frequency = 44100;
}

CtrAudio::~CtrAudio() {
	csndExit();
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

}

void CtrAudio::BGS_Pause() {

}

void CtrAudio::BGS_Resume() {

}

void CtrAudio::BGS_Stop() {

}

void CtrAudio::BGS_Fade(int fade) {

}

int CtrAudio::BGS_GetChannel() const {
	return 1;
}

void CtrAudio::ME_Play(std::string const& file, int volume, int /* pitch */, int fadein) {

}

void CtrAudio::ME_Stop() {

}

void CtrAudio::ME_Fade(int fade) {

}

void CtrAudio::SE_Play(std::string const& file, int volume, int /* pitch */) {

}

void CtrAudio::SE_Stop() {

}

void CtrAudio::Update() {

}

#endif