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
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_message.h"
#include "game_screen.h"
#include "spriteset_map.h"
#include "sprite_character.h"
#include "scene_map.h"
#include "scene_battle.h"
#include "scene_menu.h"
#include "scene_save.h"
#include "scene_load.h"
#include "scene_name.h"
#include "scene_shop.h"
#include "scene_gameover.h"
#include "scene.h"
#include "graphics.h"
#include "input.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"
#include "game_interpreter_map.h"
#include <lcf/reader_lcf.h>

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

void Game_Interpreter_Map::SetState(const lcf::rpg::SaveEventExecState& save) {
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
bool Game_Interpreter_Map::ExecuteCommand(lcf::rpg::EventCommand const& com) {
	switch (static_cast<Cmd>(com.code)) {
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
		case static_cast <Game_Interpreter::Cmd>(2002): // Cmd::EasyRpg_TriggerEventAt
			return CommandEasyRpgTriggerEventAt(com);
		default:
			return Game_Interpreter::ExecuteCommand(com);
	}
}

/**
 * Commands
 */
bool Game_Interpreter_Map::CommandRecallToLocation(lcf::rpg::EventCommand const& com) { // Code 10830
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

	int var_map_id = com.parameters[0];
	int var_x = com.parameters[1];
	int var_y = com.parameters[2];
	int map_id = Main_Data::game_variables->Get(var_map_id);
	int x = Main_Data::game_variables->Get(var_x);
	int y = Main_Data::game_variables->Get(var_y);

	auto tt = main_flag ? TeleportTarget::eForegroundTeleport : TeleportTarget::eParallelTeleport;

	Main_Data::game_player->ReserveTeleport(map_id, x, y, -1, tt);

	// Parallel events should keep on running in 2k and 2k3, unlike in later versions
	if (!main_flag)
		return true;

	index++;
	return false;
}

bool Game_Interpreter_Map::CommandEnemyEncounter(lcf::rpg::EventCommand const& com) { // code 10710
	auto& frame = GetFrame();
	auto& index = frame.current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	BattleArgs args;

	args.troop_id = ValueOrVariable(com.parameters[0], com.parameters[1]);

	switch (com.parameters[2]) {
	case 0:
		Game_Map::SetupBattle(args);
		break;
	case 1:
		args.background = ToString(com.string);

		if (Player::IsRPG2k3()) {
			args.formation = static_cast<lcf::rpg::System::BattleFormation>(com.parameters[7]);
		}
		break;
	case 2:
		args.terrain_id = com.parameters[8];
		break;
	default:
		return false;
	}
	auto escape_mode = com.parameters[3]; // 0 disallow, 1 end event processing, 2 victory/escape custom handler
	auto defeat_mode = com.parameters[4]; // 0 game over, 1 victory/defeat custom handler

	if (escape_mode == 1) {
		_state.abort_on_escape = true;
	}

	args.allow_escape = (escape_mode != 0);
	args.first_strike = com.parameters[5] != 0;

	if (Player::IsRPG2k3()) {
		args.condition = static_cast<lcf::rpg::System::BattleCondition>(com.parameters[6]);
	}

	auto indent = com.indent;
	auto continuation = [this, indent, defeat_mode](BattleResult result) {
		int sub_idx = subcommand_sentinel;

		switch (result) {
			case BattleResult::Victory:
				sub_idx = eOptionEnemyEncounterVictory;
				break;
			case BattleResult::Escape:
				sub_idx = eOptionEnemyEncounterEscape;
				if (_state.abort_on_escape) {
					return EndEventProcessing();
				}
				break;
			case BattleResult::Defeat:
				sub_idx = eOptionEnemyEncounterDefeat;
				if (defeat_mode == 0) {
					Scene::Push(std::make_shared<Scene_Gameover>());
				}
				break;
			case BattleResult::Abort:
				break;
		}

		SetSubcommandIndex(indent, sub_idx);
	};

	args.on_battle_end = continuation;

	Scene::instance->SetRequestedScene(Scene_Battle::Create(std::move(args)));

	// save game compatibility with RPG_RT
	ReserveSubcommandIndex(com.indent);

	++index;
	return false;
}

bool Game_Interpreter_Map::CommandVictoryHandler(lcf::rpg::EventCommand const& com) { // code 20710
	return CommandOptionGeneric(com, eOptionEnemyEncounterVictory, {Cmd::EscapeHandler, Cmd::DefeatHandler, Cmd::EndBattle});
}

bool Game_Interpreter_Map::CommandEscapeHandler(lcf::rpg::EventCommand const& com) { // code 20711
	return CommandOptionGeneric(com, eOptionEnemyEncounterEscape, {Cmd::DefeatHandler, Cmd::EndBattle});
}

bool Game_Interpreter_Map::CommandDefeatHandler(lcf::rpg::EventCommand const& com) { // code 20712
	return CommandOptionGeneric(com, eOptionEnemyEncounterDefeat, {Cmd::EndBattle});
}

bool Game_Interpreter_Map::CommandEndBattle(lcf::rpg::EventCommand const& /* com */) { // code 20713
	return true;
}

bool Game_Interpreter_Map::CommandOpenShop(lcf::rpg::EventCommand const& com) { // code 10720
	auto& frame = GetFrame();
	auto& index = frame.current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	bool allow_buy = false;
	bool allow_sell = false;

	switch (com.parameters[0]) {
		case 0:
			allow_buy = true;
			allow_sell = true;
			break;
		case 1:
			allow_buy = true;
			break;
		case 2:
			allow_sell = true;
			break;
		default:
			break;
	}

	auto shop_type = com.parameters[1];

	// Not used, but left here for documentation purposes
	//bool has_shop_handlers = com.parameters[2] != 0;

	std::vector<int> goods;
	for (auto it = com.parameters.begin() + 4; it < com.parameters.end(); ++it) {
		goods.push_back(*it);
	}

	auto indent = com.indent;
	auto continuation = [this, indent](bool did_transaction) {
		int sub_idx = did_transaction ? eOptionShopTransaction : eOptionShopNoTransaction;
		SetSubcommandIndex(indent, sub_idx);
	};

	auto scene = std::make_shared<Scene_Shop>(
			std::move(goods), shop_type, allow_buy, allow_sell, std::move(continuation));

	Scene::instance->SetRequestedScene(std::move(scene));

	// save game compatibility with RPG_RT
	ReserveSubcommandIndex(com.indent);

	++index;
	return false;
}

bool Game_Interpreter_Map::CommandTransaction(lcf::rpg::EventCommand const& com) { // code 20720
	return CommandOptionGeneric(com, eOptionShopTransaction, {Cmd::NoTransaction, Cmd::EndShop});
}

bool Game_Interpreter_Map::CommandNoTransaction(lcf::rpg::EventCommand const& com) { // code 20721
	return CommandOptionGeneric(com, eOptionShopNoTransaction, {Cmd::EndShop});
}

bool Game_Interpreter_Map::CommandEndShop(lcf::rpg::EventCommand const& /* com */) { // code 20722
	return true;
}

bool Game_Interpreter_Map::CommandShowInn(lcf::rpg::EventCommand const& com) { // code 10730
	int inn_type = com.parameters[0];
	int inn_price = com.parameters[1];
	// Not used, but left here for documentation purposes
	// bool has_inn_handlers = com.parameters[2] != 0;

	if (inn_price == 0) {
		if (Game_Message::IsMessageActive()) {
			return false;
		}

		// Skip prompt.
		_async_op = ContinuationShowInnStart(com.indent, 0, inn_price);
		return true;
	}

	// Emulates RPG_RT behavior (Bug?) Inn's called by parallel events
	// overwrite the current message.
	if (main_flag && !Game_Message::CanShowMessage(main_flag)) {
		return false;
	}

	PendingMessage pm(Game_Message::CommandCodeInserter);

	StringView greeting_1, greeting_2, greeting_3, accept, cancel;

	switch (inn_type) {
		case 0:
			greeting_1 = lcf::Data::terms.inn_a_greeting_1;
			greeting_2 = lcf::Data::terms.inn_a_greeting_2;
			greeting_3 = lcf::Data::terms.inn_a_greeting_3;
			accept = lcf::Data::terms.inn_a_accept;
			cancel = lcf::Data::terms.inn_a_cancel;
			break;
		case 1:
			greeting_1 = lcf::Data::terms.inn_b_greeting_1;
			greeting_2 = lcf::Data::terms.inn_b_greeting_2;
			greeting_3 = lcf::Data::terms.inn_b_greeting_3;
			accept = lcf::Data::terms.inn_b_accept;
			cancel = lcf::Data::terms.inn_b_cancel;
			break;
	}

	if (Player::IsRPG2kE()) {
		auto price_s = std::to_string(inn_price);
		pm.PushLine(
			Utils::ReplacePlaceholders(
				greeting_1,
				Utils::MakeArray('V', 'U'),
				Utils::MakeSvArray(price_s, lcf::Data::terms.gold)
			)
		);
		pm.PushLine(
			Utils::ReplacePlaceholders(
				greeting_3,
				Utils::MakeArray('V', 'U'),
				Utils::MakeSvArray(price_s, lcf::Data::terms.gold)
			)
		);
	}
	else {
		pm.PushLine(fmt::format("{} {}{} {}", greeting_1, inn_price, lcf::Data::terms.gold, greeting_2));
		pm.PushLine(ToString(greeting_3));
	}

	bool can_afford = (Main_Data::game_party->GetGold() >= inn_price);
	pm.SetChoiceResetColors(true);

	pm.PushChoice(ToString(accept), can_afford);
	pm.PushChoice(ToString(cancel));

	pm.SetShowGoldWindow(true);

	int indent = com.indent;
	pm.SetChoiceContinuation([this, indent, inn_price](int choice_result) {
			return ContinuationShowInnStart(indent, choice_result, inn_price);
			});

	// save game compatibility with RPG_RT
	ReserveSubcommandIndex(com.indent);

	Game_Message::SetPendingMessage(std::move(pm));
	_state.show_message = true;

	return true;
}

AsyncOp Game_Interpreter_Map::ContinuationShowInnStart(int indent, int choice_result, int price) {
	bool inn_stay = (choice_result == 0);

	SetSubcommandIndex(indent, inn_stay ? eOptionInnStay : eOptionInnNoStay);

	if (inn_stay) {
		Main_Data::game_party->GainGold(-price);

		return AsyncOp::MakeCallInn();
	}
	return {};
}

bool Game_Interpreter_Map::CommandStay(lcf::rpg::EventCommand const& com) { // code 20730
	return CommandOptionGeneric(com, eOptionInnStay, {Cmd::NoStay, Cmd::EndInn});
}

bool Game_Interpreter_Map::CommandNoStay(lcf::rpg::EventCommand const& com) { // code 20731
	return CommandOptionGeneric(com, eOptionInnNoStay, {Cmd::EndInn});
}

bool Game_Interpreter_Map::CommandEndInn(lcf::rpg::EventCommand const& /* com */) { // code 20732
	return true;
}

bool Game_Interpreter_Map::CommandEnterHeroName(lcf::rpg::EventCommand const& com) { // code 10740
	auto& frame = GetFrame();
	auto& index = frame.current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto actor_id = com.parameters[0];
	auto charset = com.parameters[1];
	auto use_default_name = com.parameters[2];

	auto scene = std::make_shared<Scene_Name>(actor_id, charset, use_default_name);
	Scene::instance->SetRequestedScene(std::move(scene));

	++index;
	return false;
}

bool Game_Interpreter_Map::CommandTeleport(lcf::rpg::EventCommand const& com) { // Code 10810
																		   // TODO: if in battle return true
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

	int map_id = com.parameters[0];
	int x = com.parameters[1];
	int y = com.parameters[2];

	// RPG2k3 feature
	int direction = -1;
	if (com.parameters.size() > 3 && Player::IsRPG2k3Commands()) {
		direction = com.parameters[3] - 1;
	}

	auto tt = main_flag ? TeleportTarget::eForegroundTeleport : TeleportTarget::eParallelTeleport;

	Main_Data::game_player->ReserveTeleport(map_id, x, y, direction, tt);

	// Parallel events should keep on running in 2k and 2k3, unlike in later versions
	if (!main_flag)
		return true;

	index++;
	return false;
}

bool Game_Interpreter_Map::CommandEnterExitVehicle(lcf::rpg::EventCommand const& /* com */) { // code 10840
	Main_Data::game_player->GetOnOffVehicle();

	return true;
}

bool Game_Interpreter_Map::CommandPanScreen(lcf::rpg::EventCommand const& com) { // code 11060
	int direction;
	int distance;
	int speed;
	bool waiting_pan_screen = false;

	auto& player = *Main_Data::game_player;

	switch (com.parameters[0]) {
	case 0: // Lock
		player.LockPan();
		break;
	case 1: // Unlock
		player.UnlockPan();
		break;
	case 2: // Pan
		direction = com.parameters[1];
		distance = com.parameters[2];
		// FIXME: For an "instant pan" Yume2kki passes a huge value (53) here
		// which crashes depending on the hardware
		speed = Utils::Clamp<int>(com.parameters[3], 1, 6);
		waiting_pan_screen = com.parameters[4] != 0;
		player.StartPan(direction, distance, speed);
		break;
	case 3: // Reset
		speed = Utils::Clamp<int>(com.parameters[3], 1, 6);
		waiting_pan_screen = com.parameters[4] != 0;
		player.ResetPan(speed);
		distance = std::max(
				std::abs(player.GetPanX() - player.GetTargetPanX())
				, std::abs(player.GetPanY() - player.GetTargetPanY()));
		distance /= SCREEN_TILE_SIZE;
		break;
	}

	if (waiting_pan_screen) {
		// RPG_RT uses the max wait for all pending pan commands, not just the current one.
		_state.wait_time = player.GetPanWait();
	}

	return true;
}

bool Game_Interpreter_Map::CommandShowBattleAnimation(lcf::rpg::EventCommand const& com) { // code 11210
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

bool Game_Interpreter_Map::CommandFlashSprite(lcf::rpg::EventCommand const& com) { // code 11320
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

bool Game_Interpreter_Map::CommandProceedWithMovement(lcf::rpg::EventCommand const& /* com */) { // code 11340
	_state.wait_movement = true;
	return true;
}

bool Game_Interpreter_Map::CommandHaltAllMovement(lcf::rpg::EventCommand const& /* com */) { // code 11350
	Game_Map::RemoveAllPendingMoves();
	return true;
}

bool Game_Interpreter_Map::CommandPlayMovie(lcf::rpg::EventCommand const& com) { // code 11560
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto filename = ToString(com.string);
	int pos_x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int pos_y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int res_x = com.parameters[3];
	int res_y = com.parameters[4];

	Output::Warning("Couldn't play movie: {}. Movie playback is not implemented (yet).", filename);

	Main_Data::game_screen->PlayMovie(filename, pos_x, pos_y, res_x, res_y);

	return true;
}

bool Game_Interpreter_Map::CommandOpenSaveMenu(lcf::rpg::EventCommand const& /* com */) { // code 11910
	auto& frame = GetFrame();
	auto& index = frame.current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Scene::instance->SetRequestedScene(std::make_shared<Scene_Save>());
	++index;
	return false;
}

bool Game_Interpreter_Map::CommandOpenMainMenu(lcf::rpg::EventCommand const& /* com */) { // code 11950
	auto& frame = GetFrame();
	auto& index = frame.current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Scene::instance->SetRequestedScene(std::make_shared<Scene_Menu>());
	++index;
	return false;
}

bool Game_Interpreter_Map::CommandOpenLoadMenu(lcf::rpg::EventCommand const& /* com */) {
	if (!Player::IsRPG2k3ECommands()) {
		return true;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	Scene::instance->SetRequestedScene(std::make_shared<Scene_Load>());
	++index;
	return false;
}

bool Game_Interpreter_Map::CommandToggleAtbMode(lcf::rpg::EventCommand const& /* com */) {
	if (!Player::IsRPG2k3ECommands()) {
		return true;
	}

	Main_Data::game_system->ToggleAtbMode();
	return true;
}

bool Game_Interpreter_Map::CommandEasyRpgTriggerEventAt(lcf::rpg::EventCommand const& com) {
	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[2], com.parameters[3]);

	Main_Data::game_player->TriggerEventAt(x, y);

	return true;
}
