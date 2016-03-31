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

#include "player.h"
#include "graphics.h"
#include "input.h"
#include "output.h"
#include <cstdlib>

#ifdef USE_SDL
#  include <SDL.h>
#endif

#ifdef _3DS
#include <3ds.h>
u8 isN3DS;
#include <khax.h>
#endif

#if defined(SUPPORT_AUDIO) && defined(_3DS)
	bool isDSP = false;
#endif

extern "C" int main(int argc, char* argv[]) {

	#ifdef _3DS
	
	// Starting debug console
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);
	#ifndef NO_DEBUG
	Output::Debug("Debug console started...");
	#endif
	
	#ifdef SUPPORT_AUDIO
	aptOpenSession();
	APT_SetAppCpuTimeLimit(30);
	aptCloseSession();	
	if (osGetKernelVersion() <  SYSTEM_VERSION(2,48,3)) khaxInit(); // Executing libkhax just to be sure...
	consoleClear();
	
	// Check if we already have access to csnd:SND, if not, we will perform a kernel privilege escalation
	Handle csndHandle = 0;
	#ifndef FORCE_DSP
	srvGetServiceHandleDirect(&csndHandle, "csnd:SND");
	if(csndHandle){
		Output::Debug("csnd:SND has been selected as audio service.");
		svcCloseHandle(csndHandle);
	}else{
		Output::Debug("csnd:SND is unavailable...");
	#endif
		srvGetServiceHandleDirect(&csndHandle, "dsp::DSP");
		if(csndHandle){
			Output::Debug("dsp::DSP has been selected as audio service.");
			isDSP = true;
			svcCloseHandle(csndHandle);
		}else{
			Output::Error("dsp::DSP is unavailable. Please dump a DSP firmware to use EasyRPG Player. If the problem persists, please report us the issue.");
		}
	#ifndef FORCE_DSP
	}
	#endif
	
	fsInit();
	#ifndef CITRA3DS_COMPATIBLE
	romfsInit();
	#endif
	sdmcInit();
	
	#endif
	
	hidInit();
	
	// Enable 804 Mhz mode if on N3DS
	APT_CheckNew3DS(&isN3DS);
	if(isN3DS) osSetSpeedupEnable(true);
		
	#endif
	
	Player::Init(argc, argv);
	Graphics::Init();
	Input::Init();

	Player::Run();
	
	#ifdef _3DS
	hidExit();
	gfxExit();
	sdmcExit();
	romfsExit();
	fsExit();
	#endif
	
	return EXIT_SUCCESS;
}
