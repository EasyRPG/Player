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
#include "feature.h"
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
#include "scene_equip.h"
#include "scene_item.h"
#include "scene_menu.h"
#include "scene_order.h"
#include "scene_save.h"
#include "scene_status.h"
#include "scene_skill.h"
#include "scene_load.h"
#include "scene_name.h"
#include "scene_shop.h"
#include "scene_debug.h"
#include "scene_gameover.h"
#include "scene_settings.h"
#include "scene_language.h"
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

using namespace Game_Interpreter_Shared;

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

	// When the message was created by a parallel process, close it
	if (Game_Message::IsMessageActive() && !Game_Message::GetWindow()->GetPendingMessage().IsFromForegroundInterpreter()) {
		Game_Message::GetWindow()->FinishMessageProcessing();
	}
}

bool Game_Interpreter_Map::RequestMainMenuScene(int subscreen_id, int actor_index, bool is_db_actor) {
	if (Player::game_config.patch_direct_menu.Get() && subscreen_id == -1) {
		subscreen_id = Main_Data::game_variables->Get(Player::game_config.patch_direct_menu.Get());
		actor_index = Main_Data::game_variables->Get(Player::game_config.patch_direct_menu.Get() + 1);
		// When true refers to the index of an actor, instead of a party index
		is_db_actor = (actor_index < 0);
		actor_index = std::abs(actor_index);
	}

	std::vector<Game_Actor*> actors;

	switch (subscreen_id)
	{
	case 1: // Inventory
		Scene::instance->SetRequestedScene(std::make_shared<Scene_Item>());
		return true;
	case 2: // Skills
	case 3: // Equipment
	case 4: // Status
		if (is_db_actor) {
			Game_Actor* actor = Main_Data::game_actors->GetActor(actor_index);
			if (!actor) {
				Output::Warning("RequestMainMenu: Invalid actor ID {}", actor_index);
				return false;
			}
			actors = std::vector{actor};
			actor_index = 0;
		} else {
			// 0, 1 and 5+ refer to the first actor
			if (actor_index == 0 || actor_index > 4) {
				actor_index = 1;
			}
			actor_index--;
			actors = Main_Data::game_party->GetActors();

			if (actor_index < 0 || actor_index >= static_cast<int>(actors.size())) {
				Output::Warning("RequestMainMenu: Invalid actor party member {}", actor_index);
				return false;
			}
		}

		if (subscreen_id == 2) {
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Skill>(actors, actor_index));
		}
		else if (subscreen_id == 3) {
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Equip>(actors, actor_index));
		}
		else if (subscreen_id == 4) {
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Status>(actors, actor_index));
		}
		return true;
	case 5: // Order
		if (!Feature::HasRow()) {
			break;
		}

		if (Main_Data::game_party->GetActors().size() <= 1) {
			Output::Warning("Party size must exceed '1' for 'Order' subscreen to be opened");
			return false;
		}
		else {
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Order>());
			return true;
		}
	case 6: // Settings
		Scene::instance->SetRequestedScene(std::make_shared<Scene_Settings>());
		return true;
	case 7: // Language
		if (Player::translation.HasTranslations()) {
			Scene::instance->SetRequestedScene(std::make_shared<Scene_Language>());
		}
		return true;
	}

	Scene::instance->SetRequestedScene(std::make_shared<Scene_Menu>());
	return true;
}

/**
 * Execute Command.
 */
bool Game_Interpreter_Map::ExecuteCommand(lcf::rpg::EventCommand const& com) {
	switch (static_cast<Cmd>(com.code)) {
		case Cmd::RecallToLocation:
			return CmdSetup<&Game_Interpreter_Map::CommandRecallToLocation, 3>(com);
		case Cmd::EnemyEncounter:
			if (Player::IsRPG2k()) {
				return CmdSetup<&Game_Interpreter_Map::CommandEnemyEncounter, 6>(com);
			} else {
				return CmdSetup<&Game_Interpreter_Map::CommandEnemyEncounter, 10>(com);
			}
		case Cmd::VictoryHandler:
			return CmdSetup<&Game_Interpreter_Map::CommandVictoryHandler, 0>(com);
		case Cmd::EscapeHandler:
			return CmdSetup<&Game_Interpreter_Map::CommandEscapeHandler, 0>(com);
		case Cmd::DefeatHandler:
			return CmdSetup<&Game_Interpreter_Map::CommandDefeatHandler, 0>(com);
		case Cmd::EndBattle:
			return CmdSetup<&Game_Interpreter_Map::CommandEndBattle, 0>(com);
		case Cmd::OpenShop:
			return CmdSetup<&Game_Interpreter_Map::CommandOpenShop, 4>(com);
		case Cmd::Transaction:
			return CmdSetup<&Game_Interpreter_Map::CommandTransaction, 0>(com);
		case Cmd::NoTransaction:
			return CmdSetup<&Game_Interpreter_Map::CommandNoTransaction, 0>(com);
		case Cmd::EndShop:
			return CmdSetup<&Game_Interpreter_Map::CommandEndShop, 0>(com);
		case Cmd::ShowInn:
			return CmdSetup<&Game_Interpreter_Map::CommandShowInn, 3>(com);
		case Cmd::Stay:
			return CmdSetup<&Game_Interpreter_Map::CommandStay, 0>(com);
		case Cmd::NoStay:
			return CmdSetup<&Game_Interpreter_Map::CommandNoStay, 0>(com);
		case Cmd::EndInn:
			return CmdSetup<&Game_Interpreter_Map::CommandEndInn, 0>(com);
		case Cmd::EnterHeroName:
			return CmdSetup<&Game_Interpreter_Map::CommandEnterHeroName, 3>(com);
		case Cmd::Teleport:
			return CmdSetup<&Game_Interpreter_Map::CommandTeleport, 3>(com);
		case Cmd::EnterExitVehicle:
			return CmdSetup<&Game_Interpreter_Map::CommandEnterExitVehicle, 0>(com);
		case Cmd::PanScreen:
			return CmdSetup<&Game_Interpreter_Map::CommandPanScreen, 5>(com);
		case Cmd::ShowBattleAnimation:
			return CmdSetup<&Game_Interpreter_Map::CommandShowBattleAnimation, 4>(com);
		case Cmd::FlashSprite:
			return CmdSetup<&Game_Interpreter_Map::CommandFlashSprite, 7>(com);
		case Cmd::ProceedWithMovement:
			return CmdSetup<&Game_Interpreter_Map::CommandProceedWithMovement, 0>(com);
		case Cmd::HaltAllMovement:
			return CmdSetup<&Game_Interpreter_Map::CommandHaltAllMovement, 0>(com);
		case Cmd::PlayMovie:
			return CmdSetup<&Game_Interpreter_Map::CommandPlayMovie, 5>(com);
		case Cmd::OpenSaveMenu:
			return CmdSetup<&Game_Interpreter_Map::CommandOpenSaveMenu, 0>(com);
		case Cmd::OpenMainMenu:
			return CmdSetup<&Game_Interpreter_Map::CommandOpenMainMenu, 0>(com);
		case Cmd::OpenLoadMenu:
			return CmdSetup<&Game_Interpreter_Map::CommandOpenLoadMenu, 0>(com);
		case Cmd::ToggleAtbMode:
			return CmdSetup<&Game_Interpreter_Map::CommandToggleAtbMode, 0>(com);
		case Cmd::EasyRpg_TriggerEventAt:
			return CmdSetup<&Game_Interpreter_Map::CommandEasyRpgTriggerEventAt, 4>(com);
		case Cmd::EasyRpg_WaitForSingleMovement:
			return CmdSetup<&Game_Interpreter_Map::CommandEasyRpgWaitForSingleMovement, 6>(com);
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
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

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
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

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
	// bool has_shop_handlers = com.parameters[2] != 0;

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
	pm.SetFromForegroundInterpreter(main_flag);

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
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

	auto actor_id = com.parameters[0];
	auto charset = com.parameters[1];
	auto use_default_name = com.parameters[2];

	Game_Actor* actor = Main_Data::game_actors->GetActor(actor_id);
	if (!actor) {
		Output::Warning("EnterHeroName: Invalid actor ID {}", actor_id);
		return true;
	}

	auto scene = std::make_shared<Scene_Name>(*actor, charset, use_default_name);
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

	if (Player::IsPatchManiac() && com.parameters.size() > 5) {
		// Pixel scrolling with h/v offsets
		bool centered = false; // absolute from default pan (centered on hero)
		bool relative = false; // relative to current camera
		int h = ValueOrVariableBitfield(com, 1, 0, 2);
		int v = ValueOrVariableBitfield(com, 1, 1, 3);
		waiting_pan_screen = (com.parameters[4] & 0x01) != 0;
		speed = ValueOrVariableBitfield(com, 1, 2, 5);
		switch (com.parameters[0]) {
		case 4: // Relative Pixel Pan (speed)
			centered = false;
			relative = true;
			player.StartPixelPan(h, v, speed, false, centered, relative);
			break;
		case 5: // Relative Pixel Pan (interpolated)
			centered = false;
			relative = true;
			player.StartPixelPan(h, v, speed, true, centered, relative);
			break;
		case 6: // Absolute Pixel Pan (speed)
			centered = (com.parameters[4] & 0x02) != 0;
			relative = (com.parameters[4] & 0x04) != 0;
			player.StartPixelPan(h, v, speed, false, centered, relative);
			break;
		case 7: // Absolute Pixel Pan (interpolated)
			centered = (com.parameters[4] & 0x02) != 0;
			relative = (com.parameters[4] & 0x04) != 0;
			player.StartPixelPan(h, v, speed, true, centered, relative);
			break;
		}
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

	Game_Character* chara = GetCharacter(evt_id, "ShowBattleAnimation");
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
	int event_id = ValueOrVariableBitfield(com, 7, 0, 0);
	int r = ValueOrVariableBitfield(com, 7, 1, 1);
	int g = ValueOrVariableBitfield(com, 7, 2, 2);
	int b = ValueOrVariableBitfield(com, 7, 3, 3);
	int p = ValueOrVariableBitfield(com, 7, 4, 4);

	int tenths = com.parameters[5];
	bool wait = com.parameters[6] > 0;
	Game_Character* event = GetCharacter(event_id, "FlashSprite");

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

bool Game_Interpreter_Map::CommandOpenSaveMenu(lcf::rpg::EventCommand const& com) { // code 11910
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

	Scene::instance->SetRequestedScene(std::make_shared<Scene_Save>());

	int current_system_function = 0;
	if (com.parameters.size() > 0) {
		current_system_function = com.parameters[0];
	}

	// Handle save menu (default behavior)
	if (!Player::IsPatchManiac() || current_system_function <= 0) {
		Scene::instance->SetRequestedScene(std::make_shared<Scene_Save>());
		++index;
		return false;
	}

	// Command "Call System Functions"
	switch (current_system_function) {
	case 1: // Load menu
		return CmdSetup<&Game_Interpreter_Map::CommandOpenLoadMenu, 100>(com);
	case 2: // Game menu
		return CmdSetup<&Game_Interpreter_Map::CommandOpenMainMenu, 100>(com);
	case 3: // Toggle fullscreen
		// TODO Implement fullscreen mode once maniacs supports it
		// const int fullscreen_mode = com.parameters[1]; // Broken in Maniac.
		return CmdSetup<&Game_Interpreter_Map::CommandToggleFullscreen, 100>(com);
	case 4: // Settings menu
		return CmdSetup<&Game_Interpreter_Map::CommandOpenVideoOptions, 100>(com);
	case 5: // Debug menu
		// const int pause_while_debugging = com.parameters[1]; // unused in our ingame debug screen.
		Scene::instance->SetRequestedScene(std::make_shared<Scene_Debug>());
		++index;
		return false;
	case 6: // License information menu
		// TODO Implement license information menu
		return true;
	case 7: // Reset game
		return CmdSetup<&Game_Interpreter_Map::CommandReturnToTitleScreen, 100>(com);
	default:
		if (Player::HasEasyRpgExtensions() && current_system_function >= 200 && current_system_function < 210) {
			const int actor_index = ValueOrVariable(com.parameters[1], com.parameters[2]);
			const bool is_db_actor = ValueOrVariable(com.parameters[3], com.parameters[4]);

			if (RequestMainMenuScene(current_system_function - 200, actor_index, is_db_actor)) {
				++index;
				return false;
			}
		} else {
			Output::Warning("CommandOpenSaveMenu: Unsupported scene {}", current_system_function);
		}
	}

	return true;
}

bool Game_Interpreter_Map::CommandOpenMainMenu(lcf::rpg::EventCommand const&) { // code 11950
	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

	int subscreen_id = -1, actor_index = 0;
	bool is_db_actor = false;

	if (RequestMainMenuScene(subscreen_id, actor_index, is_db_actor)) {
		++index;
		return false;
	}
	return true;
}

bool Game_Interpreter_Map::CommandOpenLoadMenu(lcf::rpg::EventCommand const& /* com */) {
	if (!Player::IsRPG2k3ECommands()) {
		return true;
	}

	if (Game_Message::IsMessageActive()) {
		return false;
	}

	auto& frame = GetFrame();
	auto& index = frame.current_command;

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
	if (!Player::HasEasyRpgExtensions()) {
		return true;
	}

	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[2], com.parameters[3]);

	// backwards compatible with old (shorter) command
	bool face_player = true;

	if (com.parameters.size() > 4) {
		int flags = com.parameters[4];
		face_player = (flags & 1) > 0;
	}

	Main_Data::game_player->TriggerEventAt(x, y, GetFrame().triggered_by_decision_key, face_player);

	return true;
}

bool Game_Interpreter_Map::CommandEasyRpgWaitForSingleMovement(lcf::rpg::EventCommand const& com) {
	if (!Player::HasEasyRpgExtensions()) {
		return true;
	}

	_state.easyrpg_parameters.resize(3);

	auto& event_id = _state.easyrpg_parameters[0];
	auto& failure_limit = _state.easyrpg_parameters[1];
	auto& output_var = _state.easyrpg_parameters[2];

	if (!_state.easyrpg_active) {
		event_id = ValueOrVariable(com.parameters[0], com.parameters[1]);
		failure_limit = ValueOrVariable(com.parameters[2], com.parameters[3]);
		output_var = ValueOrVariable(com.parameters[4], com.parameters[5]);
	}

	_state.easyrpg_active = false;

	Game_Character* chara = GetCharacter(event_id, "EasyRpgWaitForSingleMovement");
	if (chara == nullptr) {
		return true;
	}

	bool exists = chara->IsMoveRouteOverwritten();
	bool finished = chara->IsMoveRouteFinished();

	if (!exists || (exists && finished)) {
		if (output_var > 0) {
			// Move route inexistant or ended: Report success (1)
			Main_Data::game_variables->Set(output_var, 1);
			Game_Map::SetNeedRefresh(true);
		}
		return true;
	}

	// !finished
	if (failure_limit == 0 || chara->GetMoveFailureCount() < failure_limit) {
		// Below threshold: Yield
		_state.easyrpg_active = true;
		return false;
	}

	// Cancel move route and report abort (0)
	chara->CancelMoveRoute();
	if (output_var > 0) {
		Main_Data::game_variables->Set(output_var, 0);
		Game_Map::SetNeedRefresh(true);
	}

	return true;
}
