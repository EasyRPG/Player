/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "game_system.h"
#include "audio.h"
#include "graphics.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
namespace Game_System {
	bool save_disabled;
	bool teleport_disabled;
	bool escape_disabled;
	bool main_menu_disabled;
	unsigned int save_count;
	RPG::Music current_bgm;
	RPG::Music memorized_bgm;
	RPG::Music system_bgm[BGM_Count];
	RPG::Sound system_sfx[SFX_Count];
	Timer timers[2];
	int transitions[Transition_Count];
	std::map<int, Target> teleport_targets;
	Target escape_target;
}

static std::string system_name;

////////////////////////////////////////////////////////////
void Game_System::Init() {
	save_disabled = false;
	teleport_disabled = false;
	escape_disabled = false;
	main_menu_disabled = false;
	save_count = 0;
}

////////////////////////////////////////////////////////////
void Game_System::BgmPlay(RPG::Music bgm) {
	// RPG Maker Hack: (OFF) means play nothing
	current_bgm = bgm;
	if (!bgm.name.empty() && bgm.name != "(OFF)") {
		Audio::BGM_Play(bgm.name, bgm.volume, bgm.tempo);
	} else {
		Audio::BGM_Stop();
	}
	Graphics::FrameReset();
}

////////////////////////////////////////////////////////////
void Game_System::SePlay(RPG::Sound se) {
	if (!se.name.empty() && se.name != "(OFF)") {
		Audio::SE_Play(se.name, se.volume, se.tempo);
	}
}

////////////////////////////////////////////////////////////
std::string Game_System::GetSystemName() {
	if (system_name.empty()) {
		return Data::system.system_name;
	} else {
		return system_name;
	}
}

////////////////////////////////////////////////////////////
void Game_System::SetSystemName(std::string new_system_name) {
	system_name = new_system_name;
}

////////////////////////////////////////////////////////////
void Game_System::SetSystemBGM(int which, RPG::Music bgm) {
	system_bgm[which] = bgm;
}

////////////////////////////////////////////////////////////
void Game_System::SetSystemSE(int which, RPG::Sound sfx) {
	system_sfx[which] = sfx;
}

////////////////////////////////////////////////////////////
void Game_System::Timer::Update() {
	// TODO: if (<during battle> && !battle) return;
	if (running && value > 0)
		value--;
}

////////////////////////////////////////////////////////////
void Game_System::SetTimer(int which, int seconds) {
	Timer& timer = timers[which];
	timer.value = seconds * DEFAULT_FPS;
}

////////////////////////////////////////////////////////////
void Game_System::StartTimer(int which, bool visible, bool battle) {
	Timer& timer = timers[which];
	timer.running = true;
	timer.visible = visible;
	timer.battle = battle;
}

////////////////////////////////////////////////////////////
void Game_System::StopTimer(int which) {
	Timer& timer = timers[which];
	timer.running = false;
	timer.visible = false;
}

////////////////////////////////////////////////////////////
void Game_System::UpdateTimers() {
	timers[0].Update();
	timers[1].Update();
}

////////////////////////////////////////////////////////////
int Game_System::ReadTimer(int which) {
	Timer& timer = timers[which];
	return timer.value;
}

////////////////////////////////////////////////////////////
void Game_System::SetTransition(int which, int transition) {
	transitions[which] = transition;
}

////////////////////////////////////////////////////////////
void Game_System::AddTeleportTarget(int map_id, int x, int y, int switch_id) {
	teleport_targets[map_id] = Target(map_id, x, y, switch_id);
}

////////////////////////////////////////////////////////////
void Game_System::RemoveTeleportTarget(int map_id) {
	std::map<int,Target>::iterator it = teleport_targets.find(map_id);
	if (it != teleport_targets.end())
		teleport_targets.erase(it);
}

////////////////////////////////////////////////////////////
Game_System::Target* Game_System::GetTeleportTarget(int map_id) {
	std::map<int,Target>::iterator it = teleport_targets.find(map_id);
	if (it != teleport_targets.end())
		return &it->second;
	else
		return NULL;
}

////////////////////////////////////////////////////////////
void Game_System::SetEscapeTarget(int map_id, int x, int y, int switch_id) {
	escape_target = Target(map_id, x, y, switch_id);
}

////////////////////////////////////////////////////////////
Game_System::Target* Game_System::GetEscapeTarget() {
	return &escape_target;
}
