//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "filefinder.h"
#include "player.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"

////////////////////////////////////////////////////////////
/// Win32 Main entry point (Temporary here)
////////////////////////////////////////////////////////////
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#if 0
int __stdcall WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,  LPSTR /*lpCmdLine*/, int /*iShowCmd*/) {
	return main(__argc, __argv);
#endif
#endif


////////////////////////////////////////////////////////////
/// Main
////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    FileFinder::Init();
    Player::Init();
    Graphics::Init();
    Input::Init();
    Audio::Init();

    Player::Run();

    return EXIT_SUCCESS;
}
