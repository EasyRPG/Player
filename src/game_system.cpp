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
#include "game_system.h"
#include "audio.h"
#include "graphics.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Cosntructor
////////////////////////////////////////////////////////////
Game_System::Game_System() {

}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_System::~Game_System() {
}

////////////////////////////////////////////////////////////
/// Audio helpers
////////////////////////////////////////////////////////////
void Game_System::BgmPlay(RPG::Music* bgm) {
    //@playing_bgm = bgm
    if (bgm != NULL && bgm->name != "") {
        Audio::BGM_Play("Music\\" + bgm->name, bgm->volume, bgm->tempo);
    }
    else {
        Audio::BGM_Stop();
    }
    Graphics::FrameReset();
}
void Game_System::SePlay(RPG::Sound* se) {
    if (se != NULL && se->name != "") {
        Audio::SE_Play("Sound\\" + se->name, se->volume, se->tempo);
    }
}

////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////
std::string Game_System::GetSystemName() {
    if (system_name == "") {
        return Main_Data::data_system->windowskin_name;
    }
    else {
        return system_name;
    }
}

void Game_System::SetSystemName(std::string nsystem_name) {
    system_name = nsystem_name;
}
