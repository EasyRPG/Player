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
#include <cassert>
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

enum EnemyEncounterSubcommand {
	eOptionEnemyEncounterVictory = 0,
	eOptionEnemyEncounterEscape = 1,
	eOptionEnemyEncounterDefeat = 2,
};

enum ShopSubcommand {
	eOptionShopTransaction = 0,
	eOptionShopNoTransaction = 1,
};

enum InnSubcommand {
	eOptionInnStay = 0,
	eOptionInnNoStay = 1,
};

void Game_Interpreter_Map::SetState(const RPG::SaveEventExecState& save) {
	Clear();
	_state = save;
	_keyinput.fromSave(save);
}

void Game_Interpreter_Map::OnMapChange() {
	// When we change the map, we reset all event id's to 0.
	for (auto& frame: _state.stack) {
		frame.event_id = 0;
	}
}

/**
 * Execute Command.
 */
bool Game_Interpreter_Map::ExecuteCommand() {
	auto* frame = GetFrame();
	assert(frame);
	const auto& list = frame->commands;
	auto& index = frame->current_command;

	assert(index < (int)list.size());

	RPG::EventCommand const& com = list[index];

	switch (com.code) {
		case Cmd::RecallToLocation:
			return CommandRecallToLocation(com);
		case Cmd::EnemyEncounter:
			return CommandEnemyEncounter(com);
		case Cmd::VictoryHandler:
			return CommandVictoryHandler(com);
		case Cmd::EscapeHandler:
			return CommandEscapeHandler(com);
		case Cmd::DefeatHandler:
			return CommandDefeatHandler(com);
		case Cmd::EndBattle:
			return CommandEndBattle(com);
		case Cmd::OpenShop:
			return CommandOpenShop(com);
		case Cmd::Transaction:
			return CommandTransaction(com);
		case Cmd::NoTransaction:
			return CommandNoTransaction(com);
		case Cmd::EndShop:
			return CommandEndShop(com);
		case Cmd::ShowInn:
			return CommandShowInn(com);
		case Cmd::Stay:
			return CommandStay(com);
		case Cmd::NoStay:
			return CommandNoStay(com);
		case Cmd::EndInn:
			return CommandEndInn(com);
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
			return CommandOpenVideoOptions(com);
		default:
			return Game_Interpreter::ExecuteCommand();
	}
}

/**
 * Commands
 */
bool Game_Interpreter_Map::CommandRecallToLocation(RPG::EventCommand const& com) { // Code 10830
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	Game_Character *player = Main_Data::game_player.get();
	int var_map_id = com.parameters[0];
	int var_x = com.parameters[1];
	int var_y = com.parameters[2];
	int map_id = Game_Variables.Get(var_map_id);
	int x = Game_Variables.Get(var_x);
	int y = Game_Variables.Get(var_y);

	auto tt = main_flag ? TeleportTarget::eForegroundTeleport : TeleportTarget::eParallelTeleport;

	Main_Data::game_player->ReserveTeleport(map_id, x, y, -1, tt);

	// Parallel events should keep on running in 2k and 2k3, unlike in later versions
	if (!main_flag)
		return true;

	index++;
	return false;
}

bool Game_Interpreter_Map::CommandEnemyEncounter(RPG::EventCommand const& com) { // code 10710
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Game_Temp::battle_random_encounter = false;
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

	if (Player::IsRPG2k())
		Game_Battle::SetBattleMode(0);
	else
		Game_Battle::SetBattleMode(com.parameters[6]); // 0 normal, 1 initiative, 2 surround, 3 back attack, 4 pincer

	Game_Temp::battle_result = Game_Temp::BattleVictory;
	Scene::instance->SetRequestedScene(Scene::Battle);

	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationEnemyEncounter));

	// save game compatibility with RPG_RT
	ReserveSubcommandIndex(com.indent);

	++index;
	return false;
}

bool Game_Interpreter_Map::ContinuationEnemyEncounter(RPG::EventCommand const& com) {
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	continuation = NULL;

	int sub_idx = subcommand_sentinel;

	if (Game_Temp::battle_result == Game_Temp::BattleVictory) {
		sub_idx = eOptionEnemyEncounterVictory;
	}

	if (Game_Temp::battle_result == Game_Temp::BattleEscape) {
		sub_idx = eOptionEnemyEncounterEscape;
		//FIXME: subidx set before this anyway??
		if (Game_Temp::battle_escape_mode == 1) {
			return CommandEndEventProcessing(com);
		}
	}

	if (Game_Temp::battle_result == Game_Temp::BattleDefeat) {
		sub_idx = eOptionEnemyEncounterDefeat;
		//FIXME: subidx set before this anyway??
		if (Game_Temp::battle_defeat_mode == 0) {
			return CommandGameOver(com);
		}
	}

	SetSubcommandIndex(com.indent, sub_idx);

	return true;
}

bool Game_Interpreter_Map::CommandVictoryHandler(RPG::EventCommand const& com) { // code 20710
	return CommandOptionGeneric(com, eOptionEnemyEncounterVictory, {Cmd::EscapeHandler, Cmd::DefeatHandler, Cmd::EndBattle});
}

bool Game_Interpreter_Map::CommandEscapeHandler(RPG::EventCommand const& com) { // code 20711
	return CommandOptionGeneric(com, eOptionEnemyEncounterEscape, {Cmd::DefeatHandler, Cmd::EndBattle});
}

bool Game_Interpreter_Map::CommandDefeatHandler(RPG::EventCommand const& com) { // code 20712
	return CommandOptionGeneric(com, eOptionEnemyEncounterDefeat, {Cmd::EndBattle});
}

bool Game_Interpreter_Map::CommandEndBattle(RPG::EventCommand const& com) { // code 20713
	return true;
}

bool Game_Interpreter_Map::CommandOpenShop(RPG::EventCommand const& com) { // code 10720
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

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
	// Not used, but left here for documentation purposes
	//bool has_shop_handlers = com.parameters[2] != 0;

	Game_Temp::shop_goods.clear();
	std::vector<int32_t>::const_iterator it;
	for (it = com.parameters.begin() + 4; it < com.parameters.end(); ++it)
		Game_Temp::shop_goods.push_back(*it);

	Game_Temp::shop_transaction = false;
	Scene::instance->SetRequestedScene(Scene::Shop);
	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationOpenShop));

	// save game compatibility with RPG_RT
	ReserveSubcommandIndex(com.indent);

	++index;
	return false;
}

bool Game_Interpreter_Map::ContinuationOpenShop(RPG::EventCommand const& com) {
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	continuation = nullptr;

	int sub_idx = Game_Temp::shop_transaction ? eOptionShopTransaction : eOptionShopNoTransaction;

	SetSubcommandIndex(com.indent, sub_idx);

	return true;
}

bool Game_Interpreter_Map::CommandTransaction(RPG::EventCommand const& com) { // code 20720
	return CommandOptionGeneric(com, eOptionShopTransaction, {Cmd::NoTransaction, Cmd::EndShop});
}

bool Game_Interpreter_Map::CommandNoTransaction(RPG::EventCommand const& com) { // code 20721
	return CommandOptionGeneric(com, eOptionShopNoTransaction, {Cmd::EndShop});
}

bool Game_Interpreter_Map::CommandEndShop(RPG::EventCommand const& com) { // code 20722
	return true;
}

bool Game_Interpreter_Map::CommandShowInn(RPG::EventCommand const& com) { // code 10730
	int inn_type = com.parameters[0];
	Game_Temp::inn_price = com.parameters[1];
	// Not used, but left here for documentation purposes
	// bool has_inn_handlers = com.parameters[2] != 0;

	if (Game_Temp::inn_price == 0) {
		// Skip prompt.
		Game_Message::choice_result = 0;
		return ContinuationShowInnStart(com);
	}

	// Emulates RPG_RT behavior (Bug?) Inn's called by parallel events
	// overwrite the current message.
	if (main_flag && !Game_Message::CanShowMessage(main_flag)) {
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
					<< " " << Game_Temp::inn_price << Data::terms.gold
					<< " " << Data::terms.inn_a_greeting_2;
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
					<< " " << Game_Temp::inn_price << Data::terms.gold
					<< " " << Data::terms.inn_b_greeting_2;
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
	Game_Message::choice_reset_color = true;
	if (Main_Data::game_party->GetGold() < Game_Temp::inn_price)
		Game_Message::choice_disabled.set(0);

	Game_Temp::inn_calling = true;
	Game_Message::choice_result = 4;

	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnStart));

	// save game compatibility with RPG_RT
	ReserveSubcommandIndex(com.indent);

	return true;
}

bool Game_Interpreter_Map::ContinuationShowInnStart(RPG::EventCommand const& com) {
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	if (Game_Message::visible) {
		return false;
	}
	continuation = NULL;

	bool inn_stay = Game_Message::choice_result == 0;

	SetSubcommandIndex(com.indent, inn_stay ? eOptionInnStay : eOptionInnNoStay);

	Game_Temp::inn_calling = false;

	if (inn_stay) {
		Main_Data::game_party->GainGold(-Game_Temp::inn_price);

		_async_op = AsyncOp::MakeCallInn();
		return true;
	}

	return true;
}

bool Game_Interpreter_Map::CommandStay(RPG::EventCommand const& com) { // code 20730
	return CommandOptionGeneric(com, eOptionInnStay, {Cmd::NoStay, Cmd::EndInn});
}

bool Game_Interpreter_Map::CommandNoStay(RPG::EventCommand const& com) { // code 20731
	return CommandOptionGeneric(com, eOptionInnNoStay, {Cmd::EndInn});
}

bool Game_Interpreter_Map::CommandEndInn(RPG::EventCommand const& com) { // code 20732
	return true;
}

bool Game_Interpreter_Map::CommandEnterHeroName(RPG::EventCommand const& com) { // code 10740
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Game_Temp::hero_name_id = com.parameters[0];
	Game_Temp::hero_name_charset = com.parameters[1];

	Game_Actor *actor = Game_Actors::GetActor(Game_Temp::hero_name_id);

	if (!actor) {
		Output::Error("EnterHeroName: Invalid actor ID %d", Game_Temp::hero_name_id);
	}

	if (com.parameters[2]) {
		Game_Temp::hero_name = actor->GetName();
	} else {
		Game_Temp::hero_name.clear();
	}

	Scene::instance->SetRequestedScene(Scene::Name);
	++index;
	return false;
}

bool Game_Interpreter_Map::CommandTeleport(RPG::EventCommand const& com) { // Code 10810
																		   // TODO: if in battle return true
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	int map_id = com.parameters[0];
	int x = com.parameters[1];
	int y = com.parameters[2];

	// RPG2k3 feature
	int direction = com.parameters.size() > 3 ? com.parameters[3] - 1 : -1;

	auto tt = main_flag ? TeleportTarget::eForegroundTeleport : TeleportTarget::eParallelTeleport;

	Main_Data::game_player->ReserveTeleport(map_id, x, y, direction, tt);

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
		waiting_pan_screen = com.parameters[4] != 0;
		Game_Map::ResetPan(speed, waiting_pan_screen);
		distance = std::max(
				std::abs(Game_Map::GetPanX() - Game_Map::GetTargetPanX())
				, std::abs(Game_Map::GetPanY() - Game_Map::GetTargetPanY()));
		distance /= SCREEN_TILE_SIZE;
		break;
	}

	if (waiting_pan_screen) {
		_state.wait_time = distance * (2 << (6 - speed));
	}

	return true;
}

bool Game_Interpreter_Map::CommandShowBattleAnimation(RPG::EventCommand const& com) { // code 11210
	int animation_id = com.parameters[0];
	int evt_id = com.parameters[1];
	bool waiting_battle_anim = com.parameters[2] > 0;
	bool global = com.parameters[3] > 0;

	Game_Character* chara = GetCharacter(evt_id);
	if (chara == NULL)
		return true;

	if (evt_id == Game_Character::CharThisEvent)
		evt_id = GetThisEventId();

	int frames = Main_Data::game_screen->ShowBattleAnimation(animation_id, evt_id, global);

	if (waiting_battle_anim) {
		_state.wait_time = frames;
	}

	return true;
}

bool Game_Interpreter_Map::CommandFlashSprite(RPG::EventCommand const& com) { // code 11320
	int event_id = com.parameters[0];
	int r = com.parameters[1];
	int g = com.parameters[2];
	int b = com.parameters[3];
	int p = com.parameters[4];

	int tenths = com.parameters[5];
	bool wait = com.parameters[6] > 0;
	Game_Character* event = GetCharacter(event_id);

	if (event != NULL) {
		event->Flash(r, g, b, p, tenths * DEFAULT_FPS / 10);

		if (wait) {
			SetupWait(tenths);
		}
	}

	return true;
}

bool Game_Interpreter_Map::CommandProceedWithMovement(RPG::EventCommand const& /* com */) { // code 11340
	_state.wait_movement = true;
	return true;
}

bool Game_Interpreter_Map::CommandHaltAllMovement(RPG::EventCommand const& /* com */) { // code 11350
	Game_Map::RemoveAllPendingMoves();
	return true;
}

bool Game_Interpreter_Map::CommandPlayMovie(RPG::EventCommand const& com) { // code 11560
	if (Game_Message::IsMessageActive()) {
		return false;
	}

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
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Scene::instance->SetRequestedScene(Scene::Save);
	++index;
	return false;
}

bool Game_Interpreter_Map::CommandOpenMainMenu(RPG::EventCommand const& /* com */) { // code 11950
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Scene::instance->SetRequestedScene(Scene::Menu);
	++index;
	return false;
}

bool Game_Interpreter_Map::CommandOpenLoadMenu(RPG::EventCommand const& /* com */) {
	auto* frame = GetFrame();
	assert(frame);
	auto& index = frame->current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Scene::instance->SetRequestedScene(Scene::Load);
	++index;
	return false;
}

bool Game_Interpreter_Map::CommandToggleAtbMode(RPG::EventCommand const& /* com */) {
	Main_Data::game_data.system.atb_mode = !Main_Data::game_data.system.atb_mode;
	return true;
}

bool Game_Interpreter_Map::CommandOpenVideoOptions(RPG::EventCommand const& com) {
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Output::Warning("OpenVideoOptions: Command not supported");
	return true;
}

