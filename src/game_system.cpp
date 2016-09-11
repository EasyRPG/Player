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

// Headers
#include <fstream>
#include <functional>
#include "game_system.h"
#include "async_handler.h"
#include "audio.h"
#include "baseui.h"
#include "bitmap.h"
#include "cache.h"
#include "output.h"
#include "graphics.h"
#include "main_data.h"
#include "player.h"
#include "reader_util.h"
#include "scene_save.h"

namespace {
	FileRequestBinding music_request_id;
	std::map<std::string, FileRequestBinding> se_request_ids;
}

static RPG::SaveSystem& data = Main_Data::game_data.system;

bool bgm_pending = false;

void Game_System::Init() {
	data.Setup();
}

int Game_System::GetSaveCount() {
	return data.save_count;
}

void Game_System::BgmPlay(RPG::Music const& bgm) {
	RPG::Music previous_music = data.current_music;
	data.current_music = bgm;

	// Validate
	if (bgm.volume < 0 || bgm.volume > 100) {
		data.current_music.volume = 100;

		Output::Debug("BGM %s has invalid volume %d", bgm.name.c_str(), bgm.volume);
	}

	if (bgm.fadein < 0 || bgm.fadein > 10000) {
		data.current_music.fadein = 0;

		Output::Debug("BGM %s has invalid fadein %d", bgm.name.c_str(), bgm.fadein);
	}

	if (bgm.tempo < 50 || bgm.tempo > 200) {
		data.current_music.tempo = 100;

		Output::Debug("BGM %s has invalid tempo %d", bgm.name.c_str(), bgm.tempo);
	}

	// (OFF) means play nothing
	// A Polish RPG Maker translation overtranslated the (OFF) reserved string.
	// This particular translation uses (Brak) in editor for these cases.
	// Because RPG_RT doesn't show warnings about audios not found,
	// theses strings are ignored to prevent filling the log.
	// Though RPG_RT plays files named (Brak) is still preferred to ignore it.
	if (!bgm.name.empty() && bgm.name != "(OFF)" && bgm.name != "(Brak)") {
		// Same music: Only adjust volume and speed
		if (previous_music.name == bgm.name) {
			if (previous_music.volume != data.current_music.volume) {
				if (!bgm_pending) { // Delay if not ready
					Audio().BGM_Volume(data.current_music.volume);
				}
			}
			if (previous_music.tempo != data.current_music.tempo) {
				if (!bgm_pending) { // Delay if not ready
					Audio().BGM_Pitch(data.current_music.tempo);
				}
			}
		} else {
			Audio().BGM_Stop();
			bgm_pending = true;
			FileRequestAsync* request = AsyncHandler::RequestFile("Music", bgm.name);
			music_request_id = request->Bind(&Game_System::OnBgmReady);
			request->Start();
		}
	} else {
		BgmStop();
	}
}

void Game_System::BgmStop() {
	music_request_id = FileRequestBinding();
	data.current_music.name = "(OFF)";
	Audio().BGM_Stop();
}

void Game_System::SePlay(RPG::Sound const& se) {
	static bool ineluki_warning_shown = false;

	if (se.name.empty() || se.name == "(OFF)" || se.name == "(Brak)")
		return;

	std::string end = ".script";
	if (se.name.length() >= end.length() &&
		0 == se.name.compare(se.name.length() - end.length(), end.length(), end)) {
		if (!ineluki_warning_shown) {
			Output::Warning("This game seems to use Ineluki's key patch to support\n"
				"additional keys, mouse or scripts. Such patches are\n"
				"unsupported, so this functionality will not work!");
			ineluki_warning_shown = true;
		}
		return;
	}

	// NOTE: Yume Nikki plays hundreds of sound effects at 0% volume on startup,
	// probably for caching. This avoids "No free channels" warnings.
	if (se.volume == 0)
		return;

	int volume = se.volume;
	int tempo = se.tempo;

	// Validate
	if (se.volume < 0 || se.volume > 100) {
		Output::Debug("SE %s has invalid volume %d", se.name.c_str(), se.volume);
		volume = 100;
	}

	if (se.tempo < 50 || se.tempo > 200) {
		Output::Debug("SE %s has invalid tempo %d", se.name.c_str(), se.tempo);
		tempo = 100;
	}

	FileRequestAsync* request = AsyncHandler::RequestFile("Sound", se.name);
	se_request_ids[se.name] = request->Bind(std::bind(&Game_System::OnSeReady, std::placeholders::_1, volume, tempo));
	request->Start();
}

std::string Game_System::GetSystemName() {
	return data.graphics_name;
}

void Game_System::SetSystemName(std::string const& new_system_name) {
	data.graphics_name = new_system_name;
	Cache::SetSystemName(new_system_name);
	DisplayUi->SetBackcolor(Cache::System()->GetBackgroundColor());
}

RPG::Music& Game_System::GetSystemBGM(int which) {
	switch (which) {
		case BGM_Battle:		return data.battle_music;
		case BGM_Victory:		return data.battle_end_music;
		case BGM_Inn:			return data.inn_music;
		case BGM_Boat:			return data.boat_music;
		case BGM_Ship:			return data.ship_music;
		case BGM_Airship:		return data.airship_music;
		case BGM_GameOver:		return data.gameover_music;
	}

	return data.battle_music; // keep the compiler happy
}

RPG::Music& Game_System::GetCurrentBGM() {
	return data.current_music;
}

void Game_System::SetSystemBGM(int which, const RPG::Music& bgm) {
	GetSystemBGM(which) = bgm;
}

void Game_System::MemorizeBGM() {
	data.stored_music = data.current_music;
}

void Game_System::PlayMemorizedBGM() {
	BgmPlay(data.stored_music);
}

RPG::Sound& Game_System::GetSystemSE(int which) {
	switch (which) {
		case SFX_Cursor:		return data.cursor_se;
		case SFX_Decision:		return data.decision_se;
		case SFX_Cancel:		return data.cancel_se;
		case SFX_Buzzer:		return data.buzzer_se;
		case SFX_BeginBattle:	return data.battle_se;
		case SFX_Escape:		return data.escape_se;
		case SFX_EnemyAttacks:	return data.enemy_attack_se;
		case SFX_EnemyDamage:	return data.enemy_damaged_se;
		case SFX_AllyDamage:	return data.actor_damaged_se;
		case SFX_Evasion:		return data.dodge_se;
		case SFX_EnemyKill:		return data.enemy_death_se;
		case SFX_UseItem:		return data.item_se;
	}
	return data.cursor_se; // keep the compiler happy
}

void Game_System::SetSystemSE(int which, const RPG::Sound& sfx) {
	GetSystemSE(which) = sfx;
}

void Game_System::SetAllowTeleport(bool allow) {
	data.teleport_allowed = allow;
}

bool Game_System::GetAllowTeleport() {
	return data.teleport_allowed;
}

void Game_System::SetAllowEscape(bool allow) {
	data.escape_allowed = allow;
}

bool Game_System::GetAllowEscape() {
	return data.escape_allowed;
}

void Game_System::SetAllowSave(bool allow) {
	data.save_allowed = allow;
}

bool Game_System::GetAllowSave() {
	return data.save_allowed;
}

void Game_System::SetAllowMenu(bool allow) {
	data.menu_allowed = allow;
}

bool Game_System::GetAllowMenu() {
	return data.menu_allowed;
}

RPG::System::Stretch Game_System::GetMessageStretch() {
	return (RPG::System::Stretch)data.message_stretch;
}

void Game_System::SetMessageStretch(RPG::System::Stretch stretch) {
	data.message_stretch = stretch;
}

int Game_System::GetTransition(int which) {
	int transition = 0;

	switch (which) {
		case Transition_TeleportErase:
			transition = data.transition_out;
			break;
		case Transition_TeleportShow:
			transition = data.transition_in;
			break;
		case Transition_BeginBattleErase:
			transition = data.battle_start_fadeout;
			break;
		case Transition_BeginBattleShow:
			transition = data.battle_start_fadein;
			break;
		case Transition_EndBattleErase:
			transition = data.battle_end_fadeout;
			break;
		case Transition_EndBattleShow:
			transition = data.battle_end_fadein;
			break;
		default: assert(false && "Bad transition");
	}

	static const int fades[2][21] = {
		{
			Graphics::TransitionFadeOut,
			Graphics::TransitionRandomBlocks,
			Graphics::TransitionRandomBlocksUp,
			Graphics::TransitionRandomBlocksDown,
			Graphics::TransitionBlindClose,
			Graphics::TransitionVerticalStripesOut,
			Graphics::TransitionHorizontalStripesOut,
			Graphics::TransitionBorderToCenterOut,
			Graphics::TransitionCenterToBorderOut,
			Graphics::TransitionScrollUpOut,
			Graphics::TransitionScrollDownOut,
			Graphics::TransitionScrollLeftOut,
			Graphics::TransitionScrollRightOut,
			Graphics::TransitionVerticalDivision,
			Graphics::TransitionHorizontalDivision,
			Graphics::TransitionCrossDivision,
			Graphics::TransitionZoomIn,
			Graphics::TransitionMosaicOut,
			Graphics::TransitionWaveOut,
			Graphics::TransitionErase,
			Graphics::TransitionNone
		},
		{
			Graphics::TransitionFadeIn,
			Graphics::TransitionRandomBlocks,
			Graphics::TransitionRandomBlocksUp,
			Graphics::TransitionRandomBlocksDown,
			Graphics::TransitionBlindOpen,
			Graphics::TransitionVerticalStripesIn,
			Graphics::TransitionHorizontalStripesIn,
			Graphics::TransitionBorderToCenterIn,
			Graphics::TransitionCenterToBorderIn,
			Graphics::TransitionScrollUpIn,
			Graphics::TransitionScrollDownIn,
			Graphics::TransitionScrollLeftIn,
			Graphics::TransitionScrollRightIn,
			Graphics::TransitionVerticalCombine,
			Graphics::TransitionHorizontalCombine,
			Graphics::TransitionCrossCombine,
			Graphics::TransitionZoomOut,
			Graphics::TransitionMosaicIn,
			Graphics::TransitionWaveIn,
			Graphics::TransitionErase,
			Graphics::TransitionNone,
		}
	};

	return fades[which % 2][transition];
}

void Game_System::SetTransition(int which, int transition) {
	switch (which) {
		case Transition_TeleportErase:		data.transition_out			= transition; break;
		case Transition_TeleportShow:		data.transition_in			= transition; break;
		case Transition_BeginBattleErase:	data.battle_start_fadeout	= transition; break;
		case Transition_BeginBattleShow:	data.battle_start_fadein	= transition; break;
		case Transition_EndBattleErase:		data.battle_end_fadeout		= transition; break;
		case Transition_EndBattleShow:		data.battle_end_fadein		= transition; break;
		default: assert(false && "Bad transition");
	}
}

void Game_System::OnBgmReady(FileRequestResult* result) {
	// Take from current_music, params could have changed over time
	bgm_pending = false;
	std::string const path = FileFinder::FindMusic(result->file);
	if (path.empty()) {
		Output::Debug("Music not found: %s", result->file.c_str());
		return;
	}

	if (Utils::EndsWith(result->file, ".link")) {
		// Handle Ineluki's MP3 patch
		std::shared_ptr<std::fstream> stream = FileFinder::openUTF8(path, std::ios_base::in);
		if (!stream) {
			Output::Warning("Ineluki link read error: %s", path.c_str());
			return;
		}

		// The first line contains the path to the actual audio file to play
		std::string line = Utils::ReadLine(*stream.get());
		line = ReaderUtil::Recode(line, Player::encoding);
		
		Output::Debug("Ineluki link file: %s -> %s", path.c_str(), line.c_str());

		#ifdef EMSCRIPTEN
		Output::Warning("Ineluki MP3 patch unsupported in the web player");
		return;
		#else
		std::string line_canonical = FileFinder::MakeCanonical(line, 1);
		
		std::string ineluki_path = FileFinder::FindDefault(line_canonical);
		if (ineluki_path.empty()) {
			Output::Debug("Music not found: %s", line_canonical.c_str());
			return;
		}
		
		Audio().BGM_Play(ineluki_path, data.current_music.volume, data.current_music.tempo, data.current_music.fadein);
		
		return;
		#endif
	}
	
	Audio().BGM_Play(path, data.current_music.volume, data.current_music.tempo, data.current_music.fadein);
}

void Game_System::OnSeReady(FileRequestResult* result, int volume, int tempo) {
	auto item = se_request_ids.find(result->file);
	if (item != se_request_ids.end()) {
		se_request_ids.erase(item);
	}

	std::string const path = FileFinder::FindSound(result->file);
	if (path.empty()) {
		Output::Debug("Sound not found: %s", result->file.c_str());
		return;
	}

	Audio().SE_Play(path, volume, tempo);
}
