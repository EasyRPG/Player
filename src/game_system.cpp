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
#include "game_battle.h"
#include "audio.h"
#include "baseui.h"
#include "bitmap.h"
#include "cache.h"
#include "output.h"
#include "game_ineluki.h"
#include "transition.h"
#include "main_data.h"
#include "player.h"
#include <lcf/reader_util.h>
#include "scene_save.h"
#include "scene_map.h"
#include "utils.h"
#include "audio_secache.h"
#include "feature.h"

Game_System::Game_System()
	: dbsys(&lcf::Data::system)
{ }

void Game_System::SetupFromSave(lcf::rpg::SaveSystem save) {
	data = std::move(save);
	loaded_frame_count = data.frame_count;
}

const lcf::rpg::SaveSystem& Game_System::GetSaveData() const {
	return data;
}

bool Game_System::IsStopFilename(StringView name, Filesystem_Stream::InputStream (*find_func) (StringView), Filesystem_Stream::InputStream& found_stream) {
	if (name.empty() || name == "(OFF)") {
		found_stream = Filesystem_Stream::InputStream();
		return true;
	}

	found_stream = find_func(name);

	return !found_stream && (name.starts_with('(') && name.ends_with(')'));
}

bool Game_System::IsStopMusicFilename(StringView name, Filesystem_Stream::InputStream& found_stream) {
	return IsStopFilename(name, FileFinder::OpenMusic, found_stream);
}

bool Game_System::IsStopSoundFilename(StringView name, Filesystem_Stream::InputStream& found_stream) {
	return IsStopFilename(name, FileFinder::OpenSound, found_stream);
}

void Game_System::BgmPlay(lcf::rpg::Music const& bgm) {
	lcf::rpg::Music previous_music = data.current_music;
	data.current_music = bgm;

	// Validate
	if (bgm.volume < 0 || bgm.volume > 100) {
		data.current_music.volume = Utils::Clamp<int32_t>(bgm.volume, 0, 100);

		Output::Debug("BGM {} has invalid volume {}", bgm.name, bgm.volume);
	}

	if (bgm.fadein < 0 || bgm.fadein > 10000) {
		data.current_music.fadein = Utils::Clamp<int32_t>(bgm.fadein, 0, 10000);

		Output::Debug("BGM {} has invalid fadein {}", bgm.name, bgm.fadein);
	}

	// Normal pitch is 50 to 200 but Yume2kki uses out of range values
	if (bgm.tempo < 10 || bgm.tempo > 400) {
		data.current_music.tempo = Utils::Clamp<int32_t>(bgm.tempo, 10, 400);

		Output::Debug("BGM {} has invalid tempo {}", bgm.name, bgm.tempo);
	}

	// (OFF) means play nothing
	if (!bgm.name.empty() && bgm.name != "(OFF)") {
		// Same music: Only adjust volume and speed
		if (!data.music_stopping && previous_music.name == bgm.name) {
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
			music_request_id = request->Bind(&Game_System::OnBgmReady, this);
			request->Start();
		}
	} else {
		BgmStop();
	}

	data.music_stopping = false;
}

void Game_System::BgmStop() {
	music_request_id = FileRequestBinding();
	data.current_music.name = "(OFF)";
	Audio().BGM_Stop();
}

void Game_System::BgmFade(int duration, bool clear_current_music) {
	Audio().BGM_Fade(duration);
	if (clear_current_music) {
		data.current_music.name = "(OFF)";
	}
	data.music_stopping = true;
}

bool Game_System::BgmPlayedOnce() {
	if (Audio().BGM_PlayedOnce()) {
		if (Audio().BGM_GetType() == "wav") {
			// RPG_RT does not report looping for WAV
			return false;
		}
		return true;
	}

	return false;
}

void Game_System::SePlay(const lcf::rpg::Sound& se, bool stop_sounds) {
	if (se.name.empty()) {
		return;
	} else if (se.name == "(OFF)") {
		if (stop_sounds) {
			Audio().SE_Stop();
		}
		return;
	}

	// NOTE: Yume Nikki plays hundreds of sound effects at 0% volume on startup,
	// probably for caching. This avoids "No free channels" warnings.
	if (se.volume == 0)
		return;

	int32_t volume = se.volume;
	int32_t tempo = se.tempo;

	// Validate
	if (volume < 0 || volume > 100) {
		Output::Debug("SE {} has invalid volume {}", se.name, volume);
		volume = Utils::Clamp<int32_t>(volume, 0, 100);
	}

	// Normal pitch is 50 to 200 but Yume2kki uses out of range values
	if (tempo < 10 || tempo > 400) {
		Output::Debug("SE {} has invalid tempo {}", se.name, tempo);
		tempo = Utils::Clamp<int32_t>(se.tempo, 10, 400);
	}

	FileRequestAsync* request = AsyncHandler::RequestFile("Sound", se.name);
	lcf::rpg::Sound se_adj = se;
	se_adj.volume = volume;
	se_adj.tempo = tempo;
	se_request_ids[se.name] = request->Bind(&Game_System::OnSeReady, this, se_adj, stop_sounds);
	if (StringView(se.name).ends_with(".script")) {
		// Is a Ineluki Script File
		request->SetImportantFile(true);
	}
	request->Start();
}

void Game_System::SePlay(const lcf::rpg::Animation &animation) {
	Filesystem_Stream::InputStream stream;
	for (const auto& anim : animation.timings) {
		if (!IsStopSoundFilename(anim.se.name, stream)) {
			SePlay(anim.se);
			return;
		}
	}
}

StringView Game_System::GetSystemName() {
	return !data.graphics_name.empty() ?
		StringView(data.graphics_name) : StringView(lcf::Data::system.system_name);
}

void Game_System::OnChangeSystemGraphicReady(FileRequestResult* result) {
	Cache::SetSystemName(result->file);
	bg_color = Cache::SystemOrBlack()->GetBackgroundColor();

	Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();
	if (!scene || !scene->spriteset)
		return;
	scene->spriteset->SystemGraphicUpdated();
}

void Game_System::ReloadSystemGraphic() {
	FileRequestAsync* request = AsyncHandler::RequestFile("System", GetSystemName());
	system_request_id = request->Bind(&Game_System::OnChangeSystemGraphicReady, this);
	request->SetImportantFile(true);
	request->SetGraphicFile(true);
	request->Start();
}

void Game_System::SetSystemGraphic(const std::string& new_system_name,
		lcf::rpg::System::Stretch message_stretch,
		lcf::rpg::System::Font font) {

	bool changed = (GetSystemName() != new_system_name);

	data.graphics_name = new_system_name;
	data.message_stretch = message_stretch;
	data.font_id = font;

	if (changed) {
		ReloadSystemGraphic();
	}
}

void Game_System::ResetSystemGraphic() {
	data.graphics_name = "";
	data.message_stretch = (lcf::rpg::System::Stretch)0;
	data.font_id = (lcf::rpg::System::Font)0;

	ReloadSystemGraphic();
}


template <typename T>
static const T& GetAudio(const T& save, const T& db) {
	return save.name.empty() ? db : save;
}

template <typename T>
static void SetAudio(T& save, const T& db, T update) {
	if (update == db) {
		// RPG_RT only clears the name, but leaves the rest of the values alone
		save.name = {};
	} else {
		save = std::move(update);
	}
}

const lcf::rpg::Music& Game_System::GetSystemBGM(int which) {
	switch (which) {
		case BGM_Battle:
			return GetAudio(data.battle_music, dbsys->battle_music);
		case BGM_Victory:
			return GetAudio(data.battle_end_music, dbsys->battle_end_music);
		case BGM_Inn:
			return GetAudio(data.inn_music, dbsys->inn_music);
		case BGM_Boat:
			return GetAudio(data.boat_music, dbsys->boat_music);
		case BGM_Ship:
			return GetAudio(data.ship_music, dbsys->ship_music);
		case BGM_Airship:
			return GetAudio(data.airship_music, dbsys->airship_music);
		case BGM_GameOver:
			return GetAudio(data.gameover_music, dbsys->gameover_music);
	}

	static lcf::rpg::Music empty;
	return empty;
}

void Game_System::SetSystemBGM(int which, lcf::rpg::Music bgm) {
	switch (which) {
		case BGM_Battle:
			SetAudio(data.battle_music, dbsys->battle_music, std::move(bgm));
			break;
		case BGM_Victory:
			SetAudio(data.battle_end_music, dbsys->battle_end_music, std::move(bgm));
			break;
		case BGM_Inn:
			SetAudio(data.inn_music, dbsys->inn_music, std::move(bgm));
			break;
		case BGM_Boat:
			SetAudio(data.boat_music, dbsys->boat_music, std::move(bgm));
			break;
		case BGM_Ship:
			SetAudio(data.ship_music, dbsys->ship_music, std::move(bgm));
			break;
		case BGM_Airship:
			SetAudio(data.airship_music, dbsys->airship_music, std::move(bgm));
			break;
		case BGM_GameOver:
			SetAudio(data.gameover_music, dbsys->gameover_music, std::move(bgm));
			break;
	}
}

const lcf::rpg::Sound& Game_System::GetSystemSE(int which) {
	switch (which) {
		case SFX_Cursor:
			return GetAudio(data.cursor_se, dbsys->cursor_se);
		case SFX_Decision:
			return GetAudio(data.decision_se, dbsys->decision_se);
		case SFX_Cancel:
			return GetAudio(data.cancel_se, dbsys->cancel_se);
		case SFX_Buzzer:
			return GetAudio(data.buzzer_se, dbsys->buzzer_se);
		case SFX_BeginBattle:
			return GetAudio(data.battle_se, dbsys->battle_se);
		case SFX_Escape:
			return GetAudio(data.escape_se, dbsys->escape_se);
		case SFX_EnemyAttacks:
			return GetAudio(data.enemy_attack_se, dbsys->enemy_attack_se);
		case SFX_EnemyDamage:
			return GetAudio(data.enemy_damaged_se, dbsys->enemy_damaged_se);
		case SFX_AllyDamage:
			return GetAudio(data.actor_damaged_se, dbsys->actor_damaged_se);
		case SFX_Evasion:
			return GetAudio(data.dodge_se, dbsys->dodge_se);
		case SFX_EnemyKill:
			return GetAudio(data.enemy_death_se, dbsys->enemy_death_se);
		case SFX_UseItem:
			return GetAudio(data.item_se, dbsys->item_se);
	}

	static lcf::rpg::Sound empty;
	return empty;
}

void Game_System::SetSystemSE(int which, lcf::rpg::Sound sfx) {
	switch (which) {
		case SFX_Cursor:
			SetAudio(data.cursor_se, dbsys->cursor_se, std::move(sfx));
			break;
		case SFX_Decision:
			SetAudio(data.decision_se, dbsys->decision_se, std::move(sfx));
			break;
		case SFX_Cancel:
			SetAudio(data.cancel_se, dbsys->cancel_se, std::move(sfx));
			break;
		case SFX_Buzzer:
			SetAudio(data.buzzer_se, dbsys->buzzer_se, std::move(sfx));
			break;
		case SFX_BeginBattle:
			SetAudio(data.battle_se, dbsys->battle_se, std::move(sfx));
			break;
		case SFX_Escape:
			SetAudio(data.escape_se, dbsys->escape_se, std::move(sfx));
			break;
		case SFX_EnemyAttacks:
			SetAudio(data.enemy_attack_se, dbsys->enemy_attack_se, std::move(sfx));
			break;
		case SFX_EnemyDamage:
			SetAudio(data.enemy_damaged_se, dbsys->enemy_damaged_se, std::move(sfx));
			break;
		case SFX_AllyDamage:
			SetAudio(data.actor_damaged_se, dbsys->actor_damaged_se, std::move(sfx));
			break;
		case SFX_Evasion:
			SetAudio(data.dodge_se, dbsys->dodge_se, std::move(sfx));
			break;
		case SFX_EnemyKill:
			SetAudio(data.enemy_death_se, dbsys->enemy_death_se, std::move(sfx));
			break;
		case SFX_UseItem:
			SetAudio(data.item_se, dbsys->item_se, std::move(sfx));
			break;
	}
}

lcf::rpg::System::Stretch Game_System::GetMessageStretch() {
	return static_cast<lcf::rpg::System::Stretch>(!data.graphics_name.empty()
		? data.message_stretch
		: lcf::Data::system.message_stretch);
}

lcf::rpg::System::Font Game_System::GetFontId() {
	return static_cast<lcf::rpg::System::Font>(!data.graphics_name.empty()
		? data.font_id
		: lcf::Data::system.font_id);
}

Transition::Type Game_System::GetTransition(int which) {
	int transition = 0;

	auto get = [&](int local, int db) {
		return local >= 0 ? local : db;
	};

	switch (which) {
		case Transition_TeleportErase:
			transition = get(data.transition_out, lcf::Data::system.transition_out);
			break;
		case Transition_TeleportShow:
			transition = get(data.transition_in, lcf::Data::system.transition_in);
			break;
		case Transition_BeginBattleErase:
			transition = get(data.battle_start_fadeout, lcf::Data::system.battle_start_fadeout);
			break;
		case Transition_BeginBattleShow:
			transition = get(data.battle_start_fadein, lcf::Data::system.battle_start_fadein);
			break;
		case Transition_EndBattleErase:
			transition = get(data.battle_end_fadeout, lcf::Data::system.battle_end_fadeout);
			break;
		case Transition_EndBattleShow:
			transition = get(data.battle_end_fadein, lcf::Data::system.battle_end_fadein);
			break;
		default: assert(false && "Bad transition");
	}

	constexpr int num_types = 21;

	if (transition < 0 || transition >= num_types) {
		Output::Warning("Invalid transition value {}", transition);
		transition = Utils::Clamp(transition, 0, num_types - 1);
	}

	constexpr Transition::Type fades[2][num_types] = {
		{
			Transition::TransitionFadeOut,
			Transition::TransitionRandomBlocks,
			Transition::TransitionRandomBlocksDown,
			Transition::TransitionRandomBlocksUp,
			Transition::TransitionBlindClose,
			Transition::TransitionVerticalStripesOut,
			Transition::TransitionHorizontalStripesOut,
			Transition::TransitionBorderToCenterOut,
			Transition::TransitionCenterToBorderOut,
			Transition::TransitionScrollUpOut,
			Transition::TransitionScrollDownOut,
			Transition::TransitionScrollLeftOut,
			Transition::TransitionScrollRightOut,
			Transition::TransitionVerticalDivision,
			Transition::TransitionHorizontalDivision,
			Transition::TransitionCrossDivision,
			Transition::TransitionZoomIn,
			Transition::TransitionMosaicOut,
			Transition::TransitionWaveOut,
			Transition::TransitionCutOut,
			Transition::TransitionNone
		},
		{
			Transition::TransitionFadeIn,
			Transition::TransitionRandomBlocks,
			Transition::TransitionRandomBlocksDown,
			Transition::TransitionRandomBlocksUp,
			Transition::TransitionBlindOpen,
			Transition::TransitionVerticalStripesIn,
			Transition::TransitionHorizontalStripesIn,
			Transition::TransitionBorderToCenterIn,
			Transition::TransitionCenterToBorderIn,
			Transition::TransitionScrollUpIn,
			Transition::TransitionScrollDownIn,
			Transition::TransitionScrollLeftIn,
			Transition::TransitionScrollRightIn,
			Transition::TransitionVerticalCombine,
			Transition::TransitionHorizontalCombine,
			Transition::TransitionCrossCombine,
			Transition::TransitionZoomOut,
			Transition::TransitionMosaicIn,
			Transition::TransitionWaveIn,
			Transition::TransitionCutIn,
			Transition::TransitionNone,
		}
	};

	return fades[which % 2][transition];
}

void Game_System::SetTransition(int which, int transition) {
	auto set = [&](int t, int db) {
		return t != db ? t : -1;
	};
	switch (which) {
		case Transition_TeleportErase:
			data.transition_out = set(transition, lcf::Data::system.transition_out);
			break;
		case Transition_TeleportShow:
			data.transition_in = set(transition, lcf::Data::system.transition_in);
			break;
		case Transition_BeginBattleErase:
			data.battle_start_fadeout = set(transition, lcf::Data::system.battle_start_fadeout);
			break;
		case Transition_BeginBattleShow:
			data.battle_start_fadein = set(transition, lcf::Data::system.battle_start_fadein);
			break;
		case Transition_EndBattleErase:
			data.battle_end_fadeout = set(transition, lcf::Data::system.battle_end_fadeout);
			break;
		case Transition_EndBattleShow:
			data.battle_end_fadein = set(transition, lcf::Data::system.battle_end_fadein);
			break;
		default: assert(false && "Bad transition");
	}
}

void Game_System::OnBgmReady(FileRequestResult* result) {
	// Take from current_music, params could have changed over time
	bgm_pending = false;

	Filesystem_Stream::InputStream stream;
	if (IsStopMusicFilename(result->file, stream)) {
		Audio().BGM_Stop();
		return;
	} else if (!stream) {
		Output::Debug("Music not found: {}", result->file);
		return;
	}

	if (StringView(result->file).ends_with(".link")) {
		// Handle Ineluki's MP3 patch
		if (!stream) {
			Output::Warning("Ineluki MP3: Link read error: {}", stream.GetName());
			return;
		}

		// The first line contains the path to the actual audio file to play
		std::string line;
		if (!Utils::ReadLine(stream, line)) {
			Output::Warning("Ineluki MP3: Link file is empty: {}", stream.GetName());
			return;
		}
		line = lcf::ReaderUtil::Recode(line, Player::encoding);

		Output::Debug("Ineluki MP3: Link file: {} -> {}", stream.GetName(), line);
		std::string line_canonical = FileFinder::MakeCanonical(line, 1);

		// Needs another Async roundtrip
		bgm_pending = true;
		FileRequestAsync *request = AsyncHandler::RequestFile(line_canonical);
		music_request_id = request->Bind(&Game_System::OnBgmInelukiReady, this);
		request->Start();
		return;
	}

	Audio().BGM_Play(std::move(stream), data.current_music.volume, data.current_music.tempo, data.current_music.fadein);
}

void Game_System::OnBgmInelukiReady(FileRequestResult* result) {
	bgm_pending = false;
	Audio().BGM_Play(FileFinder::Game().OpenFile(result->file), data.current_music.volume, data.current_music.tempo, data.current_music.fadein);
}

void Game_System::OnSeReady(FileRequestResult* result, lcf::rpg::Sound se, bool stop_sounds) {
	auto item = se_request_ids.find(result->file);
	if (item != se_request_ids.end()) {
		se_request_ids.erase(item);
	}

	if (Player::IsPatchKeyPatch() && StringView(result->file).ends_with(".script")) {
		// Is a Ineluki Script File
		Main_Data::game_ineluki->Execute(se);
		return;
	}

	auto se_cache = AudioSeCache::GetCachedSe(result->file);
	if (!se_cache) {
		Filesystem_Stream::InputStream stream;
		if (IsStopSoundFilename(result->file, stream)) {
			if (stop_sounds) {
				Audio().SE_Stop();
			}
			return;
		} else if (!stream) {
			Output::Debug("Sound not found: {}", result->file);
			return;
		}
		se_cache = AudioSeCache::Create(std::move(stream), result->file);
	}

	if (!se_cache) {
		Output::Warning("Sound {}: Format not supported", result->file);
		return;
	}

	Audio().SE_Play(std::move(se_cache), se.volume, se.tempo);
}

bool Game_System::IsMessageTransparent() {
	if (Feature::HasRpg2kBattleSystem() && Game_Battle::IsBattleRunning()) {
		return false;
	}

	return data.message_transparent != 0;
}

