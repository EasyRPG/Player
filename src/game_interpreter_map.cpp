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
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "audio.h"
#include "game_map.h"
#include "game_battle.h"
#include "game_event.h"
#include "game_player.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_message.h"
#include "game_picture.h"
#include "spriteset_map.h"
#include "sprite_character.h"
#include "scene_map.h"
#include "scene.h"
#include "graphics.h"
#include "input.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"
#include "game_interpreter_map.h"
#include "reader_lcf.h"

Game_Interpreter_Map::Game_Interpreter_Map(int depth, bool main_flag) :
	Game_Interpreter(depth, main_flag) {
}

bool Game_Interpreter_Map::SetupFromSave(const std::vector<RPG::SaveEventCommands>& save, int _index) {
	if (_index < (int)save.size()) {
		event_id = save[_index].event_id;
		if (event_id != 0) {
			// When 0 the event is from a different map
			map_id = Game_Map::GetMapId();
		}
		list = save[_index].commands;
		index = save[_index].current_command;
		triggered_by_decision_key = save[_index].actioned;

		child_interpreter.reset(new Game_Interpreter_Map());
		bool result = static_cast<Game_Interpreter_Map*>(child_interpreter.get())->SetupFromSave(save, _index + 1);
		if (!result) {
			child_interpreter.reset();
		}
		return true;
	}
	return false;
}

// Taken from readers because a kitten is killed when reader_structs is included
static int GetEventCommandSize(const std::vector<RPG::EventCommand>& commands) {
	std::vector<RPG::EventCommand>::const_iterator it;

	int result = 0;
	for (it = commands.begin(); it != commands.end(); ++it) {
		result += LcfReader::IntSize(it->code);
		result += LcfReader::IntSize(it->indent);
		// Convert string to LDB encoding
		std::string orig_string = ReaderUtil::Recode(it->string, "UTF-8", Player::encoding);
		result += LcfReader::IntSize(orig_string.size());
		result += orig_string.size();

		int count = it->parameters.size();
		result += LcfReader::IntSize(count);
		for (int i = 0; i < count; i++)
			result += LcfReader::IntSize(it->parameters[i]);
	}
	result += 4; // No idea why but then it fits

	return result;
}

std::vector<RPG::SaveEventCommands> Game_Interpreter_Map::GetSaveData() const {
	std::vector<RPG::SaveEventCommands> save;

	const Game_Interpreter_Map* save_interpreter = this;

	int i = 1;

	if (save_interpreter->list.empty()) {
		return save;
	}

	while (save_interpreter != NULL) {
		RPG::SaveEventCommands save_commands;
		save_commands.commands = save_interpreter->list;
		save_commands.current_command = save_interpreter->index;
		save_commands.commands_size = GetEventCommandSize(save_commands.commands);
		save_commands.ID = i++;
		save_commands.event_id = event_id;
		save_commands.actioned = triggered_by_decision_key;
		save.push_back(save_commands);
		save_interpreter = static_cast<Game_Interpreter_Map*>(save_interpreter->child_interpreter.get());
	}

	return save;
}

/**
 * Execute Command.
 */
bool Game_Interpreter_Map::ExecuteCommand() {
	if (index >= list.size()) {
		return CommandEnd();
	}

	RPG::EventCommand const& com = list[index];

	switch (com.code) {
		case Cmd::RecallToLocation:
			return CommandRecallToLocation(com);
		case Cmd::EnemyEncounter:
			return CommandEnemyEncounter(com);
		case Cmd::VictoryHandler:
		case Cmd::EscapeHandler:
		case Cmd::DefeatHandler:
			return SkipTo(Cmd::EndBattle);
		case Cmd::EndBattle:
			return true;
		case Cmd::OpenShop:
			return CommandOpenShop(com);
		case Cmd::Transaction:
		case Cmd::NoTransaction:
			return SkipTo(Cmd::EndShop);
		case Cmd::EndShop:
			return true;
		case Cmd::ShowInn:
			return CommandShowInn(com);
		case Cmd::Stay:
		case Cmd::NoStay:
			return SkipTo(Cmd::EndInn);
		case Cmd::EndInn:
			return true;
		case Cmd::EnterHeroName:
			return CommandEnterHeroName(com);
		case Cmd::Teleport:
			return CommandTeleport(com);
		case Cmd::EnterExitVehicle:
			return CommandEnterExitVehicle(com);
		case Cmd::PanScreen:
			return CommandPanScreen(com);
		case Cmd::ShowBattleAnimation:
			return CommandShowBattleAnimation(com);
		case Cmd::FlashSprite:
			return CommandFlashSprite(com);
		case Cmd::ProceedWithMovement:
			return CommandProceedWithMovement(com);
		case Cmd::HaltAllMovement:
			return CommandHaltAllMovement(com);
		case Cmd::PlayMovie:
			return CommandPlayMovie(com);
		case Cmd::OpenSaveMenu:
			return CommandOpenSaveMenu(com);
		case Cmd::OpenMainMenu:
			return CommandOpenMainMenu(com);
		case Cmd::OpenLoadMenu:
			return CommandOpenLoadMenu(com);
		case Cmd::ToggleAtbMode:
			return CommandToggleAtbMode(com);
		case Cmd::OpenVideoOptions:
			Output::Warning("OpenVideoOptions: Command not supported");
			return true;
		default:
			return Game_Interpreter::ExecuteCommand();
	}
}

/**
 * Commands
 */
bool Game_Interpreter_Map::CommandRecallToLocation(RPG::EventCommand const& com) { // Code 10830
	Game_Character *player = Main_Data::game_player.get();
	int var_map_id = com.parameters[0];
	int var_x = com.parameters[1];
	int var_y = com.parameters[2];
	int map_id = Game_Variables[var_map_id];
	int x = Game_Variables[var_x];
	int y = Game_Variables[var_y];

	if (map_id == Game_Map::GetMapId()) {
		player->MoveTo(x, y);
		return true;
	};

	if (Main_Data::game_player->IsTeleporting() ||
		Game_Message::visible) {
			return false;
	}

	Main_Data::game_player->ReserveTeleport(map_id, x, y);
	Main_Data::game_player->StartTeleport();

	// Parallel events should keep on running in 2k and 2k3, unlike in later versions
	if (!main_flag)
		return true;

	index++;
	return false;
}

bool Game_Interpreter_Map::CommandEnemyEncounter(RPG::EventCommand const& com) { // code 10710
	Game_Temp::battle_troop_id = ValueOrVariable(com.parameters[0],
		com.parameters[1]);
	Game_Character *player = Main_Data::game_player.get();
	Game_Battle::SetTerrainId(Game_Map::GetTerrainTag(player->GetX(), player->GetY()));

	switch (com.parameters[2]) {
	case 0:
		Game_Map::SetupBattle();
		break;
	case 1:
		Game_Temp::battle_background = com.string;

		if (Player::IsRPG2k())
			Game_Temp::battle_formation = 0;
		else
			Game_Temp::battle_formation = com.parameters[7];
		break;
	case 2:
		Game_Battle::SetTerrainId(com.parameters[8]);
		break;
	default:
		return false;
	}
	Game_Temp::battle_escape_mode = com.parameters[3]; // 0 disallow, 1 end event processing, 2 victory/escape custom handler
	Game_Temp::battle_defeat_mode = com.parameters[4]; // 0 game over, 1 victory/defeat custom handler
	Game_Temp::battle_first_strike = com.parameters[5] != 0;
	Game_Temp::battle_check_surprise_attack = false;

	if (Player::IsRPG2k())
		Game_Battle::SetBattleMode(0);
	else
		Game_Battle::SetBattleMode(com.parameters[6]); // 0 normal, 1 initiative, 2 surround, 3 back attack, 4 pincer

	Game_Temp::battle_result = Game_Temp::BattleVictory;
	Game_Temp::battle_calling = true;

	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationEnemyEncounter));
	return false;
}

bool Game_Interpreter_Map::ContinuationEnemyEncounter(RPG::EventCommand const& com) {
	continuation = NULL;

	switch (Game_Temp::battle_result) {
	case Game_Temp::BattleVictory:
		if ((Game_Temp::battle_defeat_mode == 0 && Game_Temp::battle_escape_mode != 2) || !SkipTo(Cmd::VictoryHandler, Cmd::EndBattle)) {
			index++;
			return false;
		}
		index++;
		return true;
	case Game_Temp::BattleEscape:
		switch (Game_Temp::battle_escape_mode) {
		case 0:	// disallowed - shouldn't happen
			return true;
		case 1:
			return CommandEndEventProcessing(com);
		case 2:
			if (!SkipTo(Cmd::EscapeHandler, Cmd::EndBattle)) {
				index++;
				return false;
			}
			index++;
			return true;
		default:
			return false;
		}
	case Game_Temp::BattleDefeat:
		switch (Game_Temp::battle_defeat_mode) {
		case 0:
			return CommandGameOver(com);
		case 1:
			if (!SkipTo(Cmd::DefeatHandler, Cmd::EndBattle)) {
				index++;
				return false;
			}
			index++;
			return true;
		default:
			return false;
		}
	case Game_Temp::BattleAbort:
		if (!SkipTo(Cmd::EndBattle)) {
			index++;
			return false;
		}
		index++;
		return true;
	default:
		return false;
	}
}

bool Game_Interpreter_Map::CommandOpenShop(RPG::EventCommand const& com) { // code 10720

	switch (com.parameters[0]) {
		case 0:
			Game_Temp::shop_buys = true;
			Game_Temp::shop_sells = true;
			break;
		case 1:
			Game_Temp::shop_buys = true;
			Game_Temp::shop_sells = false;
			break;
		case 2:
			Game_Temp::shop_buys = false;
			Game_Temp::shop_sells = true;
			break;
		default:
			return false;
	}

	Game_Temp::shop_type = com.parameters[1];
	Game_Temp::shop_handlers = com.parameters[2] != 0;

	Game_Temp::shop_goods.clear();
	std::vector<int32_t>::const_iterator it;
	for (it = com.parameters.begin() + 4; it < com.parameters.end(); ++it)
		Game_Temp::shop_goods.push_back(*it);

	Game_Temp::shop_transaction = false;
	Game_Temp::shop_calling = true;
	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationOpenShop));
	return false;
}

bool Game_Interpreter_Map::ContinuationOpenShop(RPG::EventCommand const& /* com */) {
	continuation = nullptr;
	if (!Game_Temp::shop_handlers) {
		index++;
		return true;
	}

	if (!SkipTo(Game_Temp::shop_transaction
				? Cmd::Transaction
				: Cmd::NoTransaction,
				Cmd::EndShop)) {
		return false;
	}

	index++;
	return true;
}

bool Game_Interpreter_Map::CommandShowInn(RPG::EventCommand const& com) { // code 10730
	int inn_type = com.parameters[0];
	Game_Temp::inn_price = com.parameters[1];
	Game_Temp::inn_handlers = com.parameters[2] != 0;

	if (Game_Temp::inn_price == 0) {
		// Skip prompt.
		Game_Message::choice_result = 0;
		SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnStart));
		return false;
	}

	Game_Message::message_waiting = true;

	Game_Message::texts.clear();

	std::ostringstream out;

	switch (inn_type) {
		case 0:
			if (Player::IsRPG2kE()) {
				out << Game_Temp::inn_price;
				Game_Message::texts.push_back(
					Utils::ReplacePlaceholders(
						Data::terms.inn_a_greeting_1,
						{'V', 'U'},
						{out.str(), Data::terms.gold}
					)
				);
				Game_Message::texts.push_back(
					Utils::ReplacePlaceholders(
						Data::terms.inn_a_greeting_3,
						{'V', 'U'},
						{out.str(), Data::terms.gold}
					)
				);
			}
			else {
				out << Data::terms.inn_a_greeting_1
					<< " " << Game_Temp::inn_price
					<< " " << Data::terms.gold
					<< Data::terms.inn_a_greeting_2;
				Game_Message::texts.push_back(out.str());
				Game_Message::texts.push_back(Data::terms.inn_a_greeting_3);
			}
			break;
		case 1:
			if (Player::IsRPG2kE()) {
				out << Game_Temp::inn_price;
				Game_Message::texts.push_back(
					Utils::ReplacePlaceholders(
						Data::terms.inn_b_greeting_1,
						{'V', 'U'},
						{out.str(), Data::terms.gold}
					)
				);
				Game_Message::texts.push_back(
					Utils::ReplacePlaceholders(
						Data::terms.inn_b_greeting_3,
						{'V', 'U'},
						{out.str(), Data::terms.gold}
					)
				);
			}
			else {
				out << Data::terms.inn_b_greeting_1
					<< " " << Game_Temp::inn_price
					<< " " << Data::terms.gold
					<< Data::terms.inn_b_greeting_2;
				Game_Message::texts.push_back(out.str());
				Game_Message::texts.push_back(Data::terms.inn_b_greeting_3);
			}
			break;
		default:
			return false;
	}

	Game_Message::choice_start = Game_Message::texts.size();

	switch (inn_type) {
		case 0:
			Game_Message::texts.push_back(Data::terms.inn_a_accept);
			Game_Message::texts.push_back(Data::terms.inn_a_cancel);
			break;
		case 1:
			Game_Message::texts.push_back(Data::terms.inn_b_accept);
			Game_Message::texts.push_back(Data::terms.inn_b_cancel);
			break;
		default:
			return false;
	}

	Game_Message::choice_max = 2;
	Game_Message::choice_disabled.reset();
	if (Main_Data::game_party->GetGold() < Game_Temp::inn_price)
		Game_Message::choice_disabled.set(0);

	Game_Temp::inn_calling = true;
	Game_Message::choice_result = 4;

	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnStart));
	return false;
}

bool Game_Interpreter_Map::ContinuationShowInnStart(RPG::EventCommand const& /* com */) {
	if (Game_Message::visible) {
		return false;
	}
	continuation = NULL;

	bool inn_stay = Game_Message::choice_result == 0;

	Game_Temp::inn_calling = false;

	if (inn_stay) {
		Main_Data::game_party->GainGold(-Game_Temp::inn_price);

		// Full heal
		std::vector<Game_Actor*> actors = Main_Data::game_party->GetActors();
		for (Game_Actor* actor : actors) {
			actor->ChangeHp(actor->GetMaxHp());
			actor->SetSp(actor->GetMaxSp());
			actor->RemoveAllStates();
		}
		Graphics::GetTransition().Init(Transition::TransitionFadeOut, Scene::instance.get(), 36, true);
		Game_System::BgmFade(800);
		SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnContinue));
		return false;
	}

	if (Game_Temp::inn_handlers)
		SkipTo(Cmd::NoStay, Cmd::EndInn);
	index++;
	return true;
}

bool Game_Interpreter_Map::ContinuationShowInnContinue(RPG::EventCommand const& /* com */) {
	if (Graphics::IsTransitionPending())
		return false;

	const RPG::Music& bgm_inn = Game_System::GetSystemBGM(Game_System::BGM_Inn);
	// FIXME: Abusing before_battle_music (Which is unused when calling an Inn)
	// Is there also before_inn_music in the savegame?
	Main_Data::game_data.system.before_battle_music = Game_System::GetCurrentBGM();

	Game_System::BgmPlay(bgm_inn);

	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnFinish));

	return false;
}

bool Game_Interpreter_Map::ContinuationShowInnFinish(RPG::EventCommand const& /* com */) {
	if (Graphics::IsTransitionPending())
		return false;

	const RPG::Music& bgm_inn = Game_System::GetSystemBGM(Game_System::BGM_Inn);
	if (bgm_inn.name.empty() ||
		bgm_inn.name == "(OFF)" ||
		bgm_inn.name == "(Brak)" ||
		!Audio().BGM_IsPlaying() ||
		Audio().BGM_PlayedOnce()) {

		Game_System::BgmStop();
		continuation = NULL;
		Graphics::GetTransition().Init(Transition::TransitionFadeIn, Scene::instance.get(), 36, false);
		Game_System::BgmPlay(Main_Data::game_data.system.before_battle_music);

		if (Game_Temp::inn_handlers)
			SkipTo(Cmd::Stay, Cmd::EndInn);
		index++;
		return false;
	}

	return false;
}

bool Game_Interpreter_Map::CommandEnterHeroName(RPG::EventCommand const& com) { // code 10740
	Game_Temp::hero_name_id = com.parameters[0];
	Game_Temp::hero_name_charset = com.parameters[1];

	if (com.parameters[2] != 0) {
		Game_Actor *actor = Game_Actors::GetActor(Game_Temp::hero_name_id);

		if (!actor) {
			Output::Warning("EnterHeroName: Invalid actor ID %d", Game_Temp::hero_name_id);
			Game_Temp::hero_name.clear();
		} else {
			Game_Temp::hero_name = actor->GetName();
		}
	} else {
		Game_Temp::hero_name.clear();
	}

	Game_Temp::name_calling = true;
	return true;
}

bool Game_Interpreter_Map::CommandTeleport(RPG::EventCommand const& com) { // Code 10810
																		   // TODO: if in battle return true
	if (Main_Data::game_player->IsTeleporting() || Game_Temp::transition_processing ||
		Game_Message::visible) {
		return false;
	}

	int map_id = com.parameters[0];
	int x = com.parameters[1];
	int y = com.parameters[2];

	// RPG2k3 feature
	int direction = com.parameters.size() > 3 ? com.parameters[3] - 1 : -1;

	Main_Data::game_player->ReserveTeleport(map_id, x, y, direction);
	Main_Data::game_player->StartTeleport();

	// Parallel events should keep on running in 2k and 2k3, unlike in later versions
	if (!main_flag)
		return true;

	index++;
	return false;
}

bool Game_Interpreter_Map::CommandEnterExitVehicle(RPG::EventCommand const& /* com */) { // code 10840
	Main_Data::game_player->GetOnOffVehicle();

	return true;
}

bool Game_Interpreter_Map::CommandPanScreen(RPG::EventCommand const& com) { // code 11060
	int direction;
	int distance;
	int speed;
	bool waiting_pan_screen = false;

	switch (com.parameters[0]) {
	case 0: // Lock
		Game_Map::LockPan();
		break;
	case 1: // Unlock
		Game_Map::UnlockPan();
		break;
	case 2: // Pan
		direction = com.parameters[1];
		distance = com.parameters[2];
		speed = com.parameters[3];
		waiting_pan_screen = com.parameters[4] != 0;
		Game_Map::StartPan(direction, distance, speed, waiting_pan_screen);
		break;
	case 3: // Reset
		speed = com.parameters[3];
		distance = std::max(std::abs(Game_Map::GetPanX()), std::abs(Game_Map::GetPanY())) / SCREEN_TILE_WIDTH;
		waiting_pan_screen = com.parameters[4] != 0;
		Game_Map::ResetPan(speed, waiting_pan_screen);
		break;
	}

	if (waiting_pan_screen)
		wait_count = distance * (2 << (6 - speed));

	return true;
}

bool Game_Interpreter_Map::CommandShowBattleAnimation(RPG::EventCommand const& com) { // code 11210
	if (waiting_battle_anim) {
		waiting_battle_anim = Game_Map::IsBattleAnimationWaiting();
		return !waiting_battle_anim;
	}

	int animation_id = com.parameters[0];
	int evt_id = com.parameters[1];
	waiting_battle_anim = com.parameters[2] > 0;
	bool global = com.parameters[3] > 0;

	Game_Character* chara = GetCharacter(evt_id);
	if (chara == NULL)
		return true;

	if (evt_id == Game_Character::CharThisEvent)
		evt_id = event_id;

	Game_Map::ShowBattleAnimation(animation_id, evt_id, global);

	return !waiting_battle_anim;
}

bool Game_Interpreter_Map::CommandFlashSprite(RPG::EventCommand const& com) { // code 11320
	int event_id = com.parameters[0];
	Color color(com.parameters[1] * 255 / 31,
		com.parameters[2] * 255 / 31,
		com.parameters[3] * 255 / 31,
		com.parameters[4] * 255 / 31);

	int tenths = com.parameters[5];
	bool wait = com.parameters[6] > 0;
	Game_Character* event = GetCharacter(event_id);

	if (event != NULL) {
		event->Flash(color, tenths);

		if (wait)
			SetupWait(tenths);
	}

	return true;
}

bool Game_Interpreter_Map::CommandProceedWithMovement(RPG::EventCommand const& /* com */) { // code 11340
	return !Game_Map::IsAnyMovePending();
}

bool Game_Interpreter_Map::CommandHaltAllMovement(RPG::EventCommand const& /* com */) { // code 11350
	Game_Map::RemoveAllPendingMoves();
	return true;
}

bool Game_Interpreter_Map::CommandPlayMovie(RPG::EventCommand const& com) { // code 11560
	const std::string& filename = com.string;
	int pos_x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int pos_y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int res_x = com.parameters[3];
	int res_y = com.parameters[4];

	Output::Warning("Couldn't play movie: %s. Movie playback is not implemented (yet).", filename.c_str());

	Main_Data::game_screen->PlayMovie(filename, pos_x, pos_y, res_x, res_y);

	return true;
}

bool Game_Interpreter_Map::CommandOpenSaveMenu(RPG::EventCommand const& /* com */) { // code 11910
	Game_Temp::save_calling = true;
	return true;
}

bool Game_Interpreter_Map::CommandOpenMainMenu(RPG::EventCommand const& /* com */) { // code 11950
	Game_Temp::menu_calling = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter_Map::CommandOpenLoadMenu(RPG::EventCommand const& /* com */) {
	Game_Temp::load_calling = true;
	return true;
}

bool Game_Interpreter_Map::CommandToggleAtbMode(RPG::EventCommand const& /* com */) {
	Main_Data::game_data.system.atb_mode = !Main_Data::game_data.system.atb_mode;
	return true;
}
