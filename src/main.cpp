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
#include <cstdlib>

#ifdef USE_SDL
#  include <SDL.h>
#endif

#ifdef _3DS
#include <3ds.h>
u8 isN3DS;
#endif

extern "C" int main(int argc, char* argv[]) {

	#ifdef _3DS
	
	// Basic services init
	gfxInitDefault();
	hidInit();
	aptOpenSession();
	APT_SetAppCpuTimeLimit(30);
	aptCloseSession();
	
	// Enable 804 Mhz mode if on N3DS
	APT_CheckNew3DS(&isN3DS);
	if(isN3DS)
		osSetSpeedupEnable(true);
		
	#endif
	
	Player::Init(argc, argv);
	Graphics::Init();
	Input::Init();

	Player::Run();
	
	#ifdef _3DS
		hidExit();
		gfxExit();
	#endif
	
	return EXIT_SUCCESS;
}
