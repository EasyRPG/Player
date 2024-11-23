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
#include "game_actors.h"
#include "game_battle.h"
#include "game_enemyparty.h"
#include "game_interpreter_battle.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_variables.h"
#include <lcf/reader_util.h>
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "game_map.h"
#include "spriteset_battle.h"
#include <cassert>
#include "scene_battle.h"

enum BranchBattleSubcommand {
	eOptionBranchBattleElse = 1
};

enum TargetType {
	Actor,
	Member,
	Enemy,
};

// Implemented as a static map, since maniac hooks can only have one common event callback at a time.
// Subsequent calls will simply override the previous common event callback.
std::map<Game_Interpreter_Battle::ManiacBattleHookType, std::tuple<int, int>> Game_Interpreter_Battle::maniac_hooks;

void Game_Interpreter_Battle::InitBattle() {
	if (Player::IsPatchManiac()) {
		Game_Interpreter_Battle::maniac_hooks = {
			{Game_Interpreter_Battle::ManiacBattleHookType::AtbIncrement, std::make_tuple(0, 0)},
			{Game_Interpreter_Battle::ManiacBattleHookType::DamagePop, std::make_tuple(0, 0)},
			{Game_Interpreter_Battle::ManiacBattleHookType::Targetting, std::make_tuple(0, 0)},
			{Game_Interpreter_Battle::ManiacBattleHookType::SetState, std::make_tuple(0, 0)},
			{Game_Interpreter_Battle::ManiacBattleHookType::StatChange, std::make_tuple(0, 0)}
		};
	}
}

static const char* target_text[] = { "actor", "party member", "enemy" };

static const void MissingTargetWarning(const char* command_name, TargetType target_type, int target_id) {
	Output::Warning("{}: Invalid {} ID: {}", command_name, target_text[target_type], target_id);
}


// Provides a facility for battle sub-events to be run immediately
// without blocking the standard interpreter from actually processing them.
std::unique_ptr<Game_Interpreter_Battle> maniac_interpreter;

Game_Interpreter_Battle::Game_Interpreter_Battle(Span<const lcf::rpg::TroopPage> pages)
	: Game_Interpreter(true), pages(pages), executed(pages.size(), false)
{
	maniac_interpreter.reset(new Game_Interpreter_Battle());
}

Game_Interpreter_Battle::Game_Interpreter_Battle()
	: Game_Interpreter(true)
{
}

bool Game_Interpreter_Battle::AreConditionsMet(const lcf::rpg::TroopPageCondition& condition, Game_Battler* source) {
	if (!condition.flags.switch_a &&
		!condition.flags.switch_b &&
		!condition.flags.variable &&
		!condition.flags.turn &&
		!condition.flags.turn_enemy &&
		!condition.flags.turn_actor &&
		!condition.flags.fatigue &&
		!condition.flags.enemy_hp &&
		!condition.flags.actor_hp &&
		!condition.flags.command_actor
		) {
		// Pages without trigger are never run
		return false;
	}

	if (condition.flags.switch_a && !Main_Data::game_switches->Get(condition.switch_a_id))
		return false;

	if (condition.flags.switch_b && !Main_Data::game_switches->Get(condition.switch_b_id))
		return false;

	if (condition.flags.variable && !(Main_Data::game_variables->Get(condition.variable_id) >= condition.variable_value))
		return false;

	if (condition.flags.turn && !Game_Battle::CheckTurns(Game_Battle::GetTurn(), condition.turn_b, condition.turn_a))
		return false;

	if (Player::IsRPG2k3Commands() && condition.flags.turn_enemy) {
		const auto* enemy = Main_Data::game_enemyparty->GetEnemy(condition.turn_enemy_id);
		if (!enemy) {
			Output::Warning("AreConditionsMet: Invalid enemy ID {}", condition.turn_enemy_id);
			return false;
		}

		if (source && source != enemy)
			return false;
		if (!Game_Battle::CheckTurns(enemy->GetBattleTurn(), condition.turn_enemy_b, condition.turn_enemy_a))
			return false;
	}

	if (Player::IsRPG2k3Commands() && condition.flags.turn_actor) {
		const auto* actor = Main_Data::game_actors->GetActor(condition.turn_actor_id);
		if (!actor) {
			Output::Warning("AreConditionsMet: Invalid actor ID {}", condition.turn_actor_id);
			return false;
		}

		if (source && source != actor)
			return false;
		if (!Game_Battle::CheckTurns(actor->GetBattleTurn(), condition.turn_actor_b, condition.turn_actor_a))
			return false;
	}

	if (Player::IsRPG2k3Commands() && condition.flags.fatigue) {
		int fatigue = Main_Data::game_party->GetFatigue();
		if (fatigue < condition.fatigue_min || fatigue > condition.fatigue_max)
			return false;
	}

	if (condition.flags.enemy_hp) {
		const auto* enemy = Main_Data::game_enemyparty->GetEnemy(condition.enemy_id);
		if (!enemy) {
			Output::Warning("AreConditionsMet: Invalid enemy ID {}", condition.enemy_id);
			return false;
		}

		int result = 100 * enemy->GetHp() / enemy->GetMaxHp();
		if (result < condition.enemy_hp_min || result > condition.enemy_hp_max)
			return false;
	}

	if (condition.flags.actor_hp) {
		const auto* actor = Main_Data::game_actors->GetActor(condition.actor_id);
		if (!actor) {
			Output::Warning("AreConditionsMet: Invalid actor ID {}", condition.actor_id);
			return false;
		}

		int result = 100 * actor->GetHp() / actor->GetMaxHp();
		if (result < condition.actor_hp_min || result > condition.actor_hp_max)
			return false;
	}

	if (Player::IsRPG2k3Commands() && condition.flags.command_actor) {
		if (!source)
			return false;
		const auto* actor = Main_Data::game_actors->GetActor(condition.command_actor_id);
		if (!actor) {
			Output::Warning("AreConditionsMet: Invalid actor ID {}", condition.command_actor_id);
			return false;
		}

		if (source != actor)
			return false;
		if (condition.command_id != actor->GetLastBattleAction())
			return false;
	}

	return true;
}

int Game_Interpreter_Battle::ScheduleNextPage(Game_Battler* source) {
	lcf::rpg::TroopPageCondition::Flags f;
	for (auto& ff: f.flags) ff = true;

	return ScheduleNextPage(f, source);
}

static bool HasRequiredCondition(lcf::rpg::TroopPageCondition::Flags page, lcf::rpg::TroopPageCondition::Flags required) {
	for (size_t i = 0; i < page.flags.size(); ++i) {
		if (required.flags[i] && page.flags[i]) {
			return true;
		}
	}
	return false;
}

int Game_Interpreter_Battle::ScheduleNextPage(lcf::rpg::TroopPageCondition::Flags required_conditions, Game_Battler* source) {
	if (IsRunning()) {
		return 0;
	}

	for (size_t i = 0; i < pages.size(); ++i) {
		auto& page = pages[i];
		if (executed[i]
				|| !HasRequiredCondition(page.condition.flags, required_conditions)
				|| !AreConditionsMet(page.condition, source)) {
			continue;
		}
		Clear();
		Push(page.event_commands, 0);
		executed[i] = true;
		return i + 1;
	}
	return 0;
}

// Execute Command.
bool Game_Interpreter_Battle::ExecuteCommand(lcf::rpg::EventCommand const& com) {
	switch (static_cast<Cmd>(com.code)) {
		case Cmd::CallCommonEvent:
			return CommandCallCommonEvent(com);
		case Cmd::ForceFlee:
			return CommandForceFlee(com);
		case Cmd::EnableCombo:
			return CommandEnableCombo(com);
		case Cmd::ChangeMonsterHP:
			return CommandChangeMonsterHP(com);
		case Cmd::ChangeMonsterMP:
			return CommandChangeMonsterMP(com);
		case Cmd::ChangeMonsterCondition:
			return CommandChangeMonsterCondition(com);
		case Cmd::ShowHiddenMonster:
			return CommandShowHiddenMonster(com);
		case Cmd::ChangeBattleBG:
			return CommandChangeBattleBG(com);
		case Cmd::ShowBattleAnimation_B:
			return CommandShowBattleAnimation(com);
		case Cmd::TerminateBattle:
			return CommandTerminateBattle(com);
		case Cmd::ConditionalBranch_B:
			return CommandConditionalBranchBattle(com);
		case Cmd::ElseBranch_B:
			return CommandElseBranchBattle(com);
		case Cmd::EndBranch_B:
			return CommandEndBranchBattle(com);
		case Cmd::Maniac_ControlBattle:
			return CommandManiacControlBattle(com);
		case Cmd::Maniac_ControlAtbGauge:
			return CommandManiacControlAtbGauge(com);
		case Cmd::Maniac_ChangeBattleCommandEx:
			return CommandManiacChangeBattleCommandEx(com);
		case Cmd::Maniac_GetBattleInfo:
			return CommandManiacGetBattleInfo(com);
		default:
			return Game_Interpreter::ExecuteCommand(com);
	}
}

// Commands

bool Game_Interpreter_Battle::CommandCallCommonEvent(lcf::rpg::EventCommand const& com) {
	if (!Player::IsRPG2k3Commands()) {
		return true;
	}

	int evt_id = com.parameters[0];

	Game_CommonEvent* common_event = lcf::ReaderUtil::GetElement(Game_Map::GetCommonEvents(), evt_id);
	if (!common_event) {
		Output::Warning("CallCommonEvent: Can't call invalid common event {}", evt_id);
		return true;
	}

	Push(common_event);

	return true;
}

bool Game_Interpreter_Battle::CommandForceFlee(lcf::rpg::EventCommand const& com) {
	if (!Player::IsRPG2k3Commands()) {
		return true;
	}

	bool check = com.parameters[2] == 0;

	switch (com.parameters[0]) {
	case 0:
		if (!check || Game_Battle::GetBattleCondition() != lcf::rpg::System::BattleCondition_pincers) {
			this->force_flee_enabled = true;
		}
		break;
	case 1:
		if (!check || Game_Battle::GetBattleCondition() != lcf::rpg::System::BattleCondition_surround) {
			int num_escaped = 0;
			for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
				if (enemy->Exists()) {
					enemy->SetHidden(true);
					enemy->SetDeathTimer();
					++num_escaped;
				}
			}
			if (num_escaped) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Escape));
			}
		}
		break;
	case 2:
		if (!check || Game_Battle::GetBattleCondition() != lcf::rpg::System::BattleCondition_surround) {
			auto* enemy = Main_Data::game_enemyparty->GetEnemy(com.parameters[1]);
			if (!enemy) {
				Output::Warning("ForceFlee: Invalid enemy ID {}", com.parameters[1]);
				return true;
			}

			if (enemy->Exists()) {
				enemy->SetHidden(true);
				enemy->SetDeathTimer();
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Escape));
			}
		}
		break;
	}

	return true;
}

bool Game_Interpreter_Battle::CommandEnableCombo(lcf::rpg::EventCommand const& com) {
	if (!Player::IsRPG2k3Commands()) {
		return true;
	}

	int actor_id = com.parameters[0];

	if (!Main_Data::game_party->IsActorInParty(actor_id)) {
		return true;
	}

	int command_id = com.parameters[1];
	int multiple = com.parameters[2];

	Game_Actor* actor = Main_Data::game_actors->GetActor(actor_id);

	if (!actor) {
		Output::Warning("EnableCombo: Invalid actor ID {}", actor_id);
		return true;
	}

	actor->SetBattleCombo(command_id, multiple);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterHP(lcf::rpg::EventCommand const& com) {
	auto* enemy = Main_Data::game_enemyparty->GetEnemy(com.parameters[0]);
	if (!enemy) {
		Output::Warning("ChangeMonsterHP: Invalid enemy ID {}", com.parameters[0]);
		return true;
	}

	bool lose = com.parameters[1] > 0;
	bool lethal = com.parameters[4] > 0;
	int hp = enemy->GetHp();

	if (enemy->IsDead())
		return true;

	int change = 0;
	switch (com.parameters[2]) {
	case 0:
		change = com.parameters[3];
		break;
	case 1:
		change = Main_Data::game_variables->Get(com.parameters[3]);
		break;
	case 2:
		change = com.parameters[3] * hp / 100;
		break;
	}

	if (lose) {
		change = -change;
	}

	enemy->ChangeHp(change, lethal);

	auto& scene = Scene::instance;
	if (scene) {
		scene->OnEventHpChanged(enemy, change);
	}

	if (enemy->IsDead()) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyKill));
		enemy->SetDeathTimer();
	}

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterMP(lcf::rpg::EventCommand const& com) {
	auto* enemy = Main_Data::game_enemyparty->GetEnemy(com.parameters[0]);
	if (!enemy) {
		Output::Warning("ChangeMonsterMP: Invalid enemy ID {}", com.parameters[0]);
		return true;
	}

	bool lose = com.parameters[1] > 0;
	int sp = enemy->GetSp();

	int change = 0;
	switch (com.parameters[2]) {
	case 0:
		change = com.parameters[3];
		break;
	case 1:
		change = Main_Data::game_variables->Get(com.parameters[3]);
		break;
	}

	if (lose)
		change = -change;

	sp += change;

	enemy->SetSp(sp);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterCondition(lcf::rpg::EventCommand const& com) {
	auto* enemy = Main_Data::game_enemyparty->GetEnemy(com.parameters[0]);
	if (!enemy) {
		Output::Warning("ChangeMonsterCondition: Invalid enemy ID {}", com.parameters[0]);
		return true;
	}

	bool remove = com.parameters[1] > 0;
	int state_id = com.parameters[2];
	if (remove) {
		// RPG_RT BUG: Monster dissapears immediately and doesn't animate death
		enemy->RemoveState(state_id, false);
	} else {
		enemy->AddState(state_id, true);
	}
	return true;
}

bool Game_Interpreter_Battle::CommandShowHiddenMonster(lcf::rpg::EventCommand const& com) {
	auto* enemy = Main_Data::game_enemyparty->GetEnemy(com.parameters[0]);
	if (!enemy) {
		Output::Warning("ShowHiddenMonster: Invalid enemy ID {}", com.parameters[0]);
		return true;
	}

	enemy->SetHidden(false);
	return true;
}

bool Game_Interpreter_Battle::CommandChangeBattleBG(lcf::rpg::EventCommand const& com) {
	Game_Battle::ChangeBackground(ToString(com.string));
	return true;
}

bool Game_Interpreter_Battle::CommandShowBattleAnimation(lcf::rpg::EventCommand const& com) {
	int animation_id = com.parameters[0];
	int target = com.parameters[1];
	bool waiting_battle_anim = com.parameters[2] != 0;
	bool allies = false;

	if (Player::IsRPG2k3()) {
		allies = com.parameters[3] != 0;
	}

	int frames = 0;

	if (target < 0) {
		std::vector<Game_Battler*> v;

		if (allies) {
			Main_Data::game_party->GetBattlers(v);
		} else {
			Main_Data::game_enemyparty->GetBattlers(v);
		}
		auto iter = std::remove_if(v.begin(), v.end(),
				[](auto* target) { return !(target->Exists() || (target->GetType() == Game_Battler::Type_Ally && target->IsDead())); });
		v.erase(iter, v.end());

		frames = Game_Battle::ShowBattleAnimation(animation_id, v, false);
	}
	else {
		Game_Battler* battler_target = nullptr;

		if (allies) {
			// Allies counted from 1
			target -= 1;
			if (target >= 0 && target < Main_Data::game_party->GetBattlerCount()) {
				battler_target = &(*Main_Data::game_party)[target];
			}
		}
		else {
			if (target < Main_Data::game_enemyparty->GetBattlerCount()) {
				battler_target = &(*Main_Data::game_enemyparty)[target];
			}
		}

		if (battler_target) {
			frames = Game_Battle::ShowBattleAnimation(animation_id, { battler_target });
		}
	}

	if (waiting_battle_anim) {
		_state.wait_time = frames;
	}

	return true;
}

bool Game_Interpreter_Battle::CommandTerminateBattle(lcf::rpg::EventCommand const& /* com */) {
	_async_op = AsyncOp::MakeTerminateBattle(static_cast<int>(BattleResult::Abort));
	return false;
}

// Conditional branch.
bool Game_Interpreter_Battle::CommandConditionalBranchBattle(lcf::rpg::EventCommand const& com) {
	bool result = false;
	int value1, value2;

	switch (com.parameters[0]) {
		case 0:
			// Switch
			result = Main_Data::game_switches->Get(com.parameters[1]) == (com.parameters[2] == 0);
			break;
		case 1:
			// Variable
			value1 = Main_Data::game_variables->Get(com.parameters[1]);
			if (com.parameters[2] == 0) {
				value2 = com.parameters[3];
			} else {
				value2 = Main_Data::game_variables->Get(com.parameters[3]);
			}
			switch (com.parameters[4]) {
				case 0:
					// Equal to
					result = (value1 == value2);
					break;
				case 1:
					// Greater than or equal
					result = (value1 >= value2);
					break;
				case 2:
					// Less than or equal
					result = (value1 <= value2);
					break;
				case 3:
					// Greater than
					result = (value1 > value2);
					break;
				case 4:
					// Less than
					result = (value1 < value2);
					break;
				case 5:
					// Different
					result = (value1 != value2);
					break;
			}
			break;
		case 2: {
			// Hero can act
			const auto* actor = Main_Data::game_actors->GetActor(com.parameters[1]);

			if (!actor) {
				Output::Warning("ConditionalBranchBattle: Invalid actor ID {}", com.parameters[1]);
			} else {
				result = actor->CanAct();
			}
			break;
		}
		case 3: {
			// Monster can act
			const auto* enemy = Main_Data::game_enemyparty->GetEnemy(com.parameters[1]);

			if (!enemy) {
				Output::Warning("ConditionalBranchBattle: Invalid enemy ID {}", com.parameters[1]);
			} else {
				result = enemy->CanAct();
			}
			break;
		}
		case 4:
			// Monster is the current target
			if (Player::IsRPG2k3Commands()) {
				result = (targets_single_enemy && target_enemy_index == com.parameters[1]);
			}
			break;
		case 5: {
			// Hero uses the ... command
			if (Player::IsRPG2k3Commands() && current_actor_id == com.parameters[1]) {
				auto *actor = Main_Data::game_actors->GetActor(current_actor_id);
				if (actor) {
					result = actor->GetLastBattleAction() == com.parameters[2];
				}
			}
			break;
		}
	}

	int sub_idx = subcommand_sentinel;
	if (!result) {
		sub_idx = eOptionBranchBattleElse;
		SkipToNextConditional({Cmd::ElseBranch_B, Cmd::EndBranch_B}, com.indent);
	}

	SetSubcommandIndex(com.indent, sub_idx);
	return true;
}

bool Game_Interpreter_Battle::CommandElseBranchBattle(lcf::rpg::EventCommand const& com) { //code 23310
	return CommandOptionGeneric(com, eOptionBranchBattleElse, {Cmd::EndBranch_B});
}

bool Game_Interpreter_Battle::CommandEndBranchBattle(lcf::rpg::EventCommand const& /* com */) { //code 23311
	return true;
}

bool Game_Interpreter_Battle::ManiacBattleHook(ManiacBattleHookType hook_type, int var1, int var2, int var3, int var4, int var5, int var6) {
	if (!Player::IsPatchManiac()) {
		return false;
	}

	int common_event_id = std::get<0>(maniac_hooks[hook_type]);
	int variable_start_id = std::get<1>(maniac_hooks[hook_type]);

	if (common_event_id <= 0) {
		return false;
	}

	Game_CommonEvent* common_event = lcf::ReaderUtil::GetElement(Game_Map::GetCommonEvents(), common_event_id);
	if (!common_event) {
		Output::Warning("CommandManiacControlBattle: Can't call invalid common event {}", common_event_id);
		return false;
	}

	// pushes the common event to be run into the queue of events.
	maniac_interpreter->Push(common_event);

	// pushes the change variable events into the interpreters
	// event queue, so we don't run into a race condition.
	std::vector<lcf::rpg::EventCommand> pre_commands;
	for (size_t i = 0; i < 6; i++)
	{
		auto event_command = lcf::rpg::EventCommand();
		event_command.code = static_cast<int>(lcf::rpg::EventCommand::Code::ControlVars);
		event_command.parameters = lcf::DBArray<int32_t>(7);
		event_command.parameters[1] = variable_start_id + i;
		switch (i)
		{
			case 0:
				event_command.parameters[5] = var1;
				break;
			case 1:
				event_command.parameters[5] = var2;
				break;
			case 2:
				event_command.parameters[5] = var3;
				break;
			case 3:
				event_command.parameters[5] = var4;
				break;
			case 4:
				event_command.parameters[5] = var5;
				break;
			case 5:
				event_command.parameters[5] = var6;
				break;
			default:
				break;
		}
		pre_commands.push_back(event_command);
	}

	// Push is actually "push_back", so this gets added before other events.
	maniac_interpreter->Push(pre_commands, 0);

	// Necessary to start the sub-event.
	maniac_interpreter->Update();

	return true;
}

bool Game_Interpreter_Battle::ProcessManiacSubEvents() {
	// If we have sub-events we're going to update them immediately
	// until the queue is empty while making the rest of the game wait.
	if (Player::IsPatchManiac() && maniac_interpreter->IsRunning()) {
		maniac_interpreter->Update();
		return true;
	}
	return false;
}

bool Game_Interpreter_Battle::CommandManiacControlBattle(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	ManiacBattleHookType control_type_flags = static_cast<ManiacBattleHookType>(com.parameters[0]);
	int common_event_flags = com.parameters[1];
	int common_event_identifier = com.parameters[2];
	int value_reference_identifier = com.parameters[3];

	int common_event_id = ValueOrVariable(common_event_flags, common_event_identifier);

	// Sets the maniacs battle event hook to:
	// the common event id and the variable id the developer would like to use.
	std::get<0>(maniac_hooks[control_type_flags]) = common_event_identifier;
	std::get<1>(maniac_hooks[control_type_flags]) = value_reference_identifier;

	return true;
}

bool Game_Interpreter_Battle::CommandManiacControlAtbGauge(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int target_flags = com.parameters[0];
	int target_reference_flags = com.parameters[1];
	int target_reference_identifier = com.parameters[2];
	int operation_flags = com.parameters[3];
	int operand_flags = com.parameters[4];
	int value_reference_flags = com.parameters[5];
	int value_reference_identifier = com.parameters[6];

	int target_id = ValueOrVariable(target_reference_flags, target_reference_identifier);

	auto getAtbValue = [this, value_reference_flags, value_reference_identifier](int flags) {
		int value = ValueOrVariable(value_reference_flags, value_reference_identifier);

		switch (flags) {
			case 0:
				// value
				return value;
				break;
			case 1:
				// percentage
				return (int) ((double)value / 100 * Game_Battler::GetMaxAtbGauge());
				break;
			default: return 0;
		}
	};

	auto executeOperation = [getAtbValue, operand_flags, target_id](int flags, Game_Battler* battler, TargetType battler_type) {
		if (!battler) {
			MissingTargetWarning("CommandManiacControlAtbGauge", battler_type, target_id);
			return;
		}

		switch (flags) {
			case 0:
				// set
				battler->SetAtbGauge(getAtbValue(operand_flags));
				break;
			case 1:
				// add
				battler->IncrementAtbGauge(getAtbValue(operand_flags));
				break;
			case 2:
				// sub
				battler->IncrementAtbGauge(-getAtbValue(operand_flags));
				break;
		}
	};

	switch (target_flags) {
		case 0:
			// actor
			if (target_id > 0) {
				executeOperation(operation_flags, Main_Data::game_actors->GetActor(target_id), Actor);
			}
			break;
		case 1:
			// party member
			executeOperation(operation_flags, Main_Data::game_party->GetActor(target_id), Member);
			break;
		case 2:
			// entire party
			for (Game_Battler* member : Main_Data::game_party->GetActors()) {
				executeOperation(operation_flags, member, Member);
			}
			break;
		case 3:
			// troop member
			if (target_id > 0) {
				executeOperation(operation_flags, Main_Data::game_enemyparty->GetEnemy(target_id), Enemy);
			}
			break;
		case 4:
			// entire troop
			for (Game_Battler* member : Main_Data::game_enemyparty->GetEnemies()) {
				executeOperation(operation_flags, member, Enemy);
			}
			break;
	}

	return true;
}

bool Game_Interpreter_Battle::CommandManiacChangeBattleCommandEx(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	// 1 row removed
	bool actor_command_flags = com.parameters[0];

	lcf::Data::battlecommands.easyrpg_disable_row_feature = actor_command_flags;

	// 10000 lose added
	// 01000 win added
	// 00100 escape removed
	// 00010 auto removed
	// 00001 fight removed
	int party_command_flags = com.parameters[1];

	lcf::Data::system.easyrpg_battle_options.clear();
	for (size_t i = 0; i < Scene_Battle::BattleOptionType::Lose + 1; i++) {
		bool party_command_flag = party_command_flags & (1 << i);
		bool flag_is_set = i > 2;

		if (party_command_flag == flag_is_set) {
			lcf::Data::system.easyrpg_battle_options.push_back(i);
		}
	}

	auto* scene_battle = static_cast<Scene_Battle*>(Scene::instance.get());

	if (scene_battle) {
		scene_battle->CreateOptions();
	}

	return true;
}

bool Game_Interpreter_Battle::CommandManiacGetBattleInfo(lcf::rpg::EventCommand const& com) {
	if (!Player::IsPatchManiac()) {
		return true;
	}

	int target_flags = com.parameters[0];
	int target_reference_flags = com.parameters[2];
	int target_reference_identifier = com.parameters[3];
	int information_flags = com.parameters[1];
	int information_identifier = com.parameters[4];

	int target_id = ValueOrVariable(target_reference_flags, target_reference_identifier);

	auto executeOperationSingle = [information_flags, information_identifier, target_id](Game_Battler* battler, TargetType battler_type) {
		if (!battler) {
			MissingTargetWarning("CommandManiacGetBattleInfo", battler_type, target_id);
			return;
		}

		switch (information_flags)
		{
			case 0:
				// parameter buffs: attack, defense, mind, agility
				Main_Data::game_variables->Set(information_identifier, battler->GetAtkModifier());
				Main_Data::game_variables->Set(information_identifier + 1, battler->GetDefModifier());
				Main_Data::game_variables->Set(information_identifier + 2, battler->GetSpiModifier());
				Main_Data::game_variables->Set(information_identifier + 3, battler->GetAgiModifier());
				break;
			case 1:
			{
				// states: size, [...state_id]
				auto states = lcf::Data::states.size();
				Main_Data::game_variables->Set(information_identifier, states);
				for (size_t i = 0; i < states; i++)
				{
					Main_Data::game_variables->Set(information_identifier + i + 1, battler->HasState(lcf::Data::states[i].ID));
				}
				break;
			}
			case 2:
			{
				// elements: size, [...element_id]
				auto elements = lcf::Data::attributes.size();
				Main_Data::game_variables->Set(information_identifier, elements);
				for (size_t i = 0; i < elements; i++)
				{
					Main_Data::game_variables->Set(information_identifier + i + 1, battler->GetAttributeRateShift(lcf::Data::attributes[i].ID) + 1);
				}
				break;
			}
			case 3:
				// others: x, y, can move, defending, charging, appeared
				Main_Data::game_variables->Set(information_identifier, battler->GetBattlePosition().x);
				Main_Data::game_variables->Set(information_identifier + 1, battler->GetBattlePosition().y);
				Main_Data::game_variables->Set(information_identifier + 2, battler->CanAct());
				Main_Data::game_variables->Set(information_identifier + 3, battler->IsDefending());
				Main_Data::game_variables->Set(information_identifier + 4, battler->IsCharged());
				Main_Data::game_variables->Set(information_identifier + 5, !battler->IsHidden());
				break;
		}
	};

	auto executeOperationMany = [information_flags](Game_Battler* battler) {
		switch (information_flags)
		{
			case 0:
				// list of members
				return true;
			case 1:
				// list of members alive
				return !battler->IsDead();
			case 2:
				// list of members who can move
				return battler->CanAct();
		}

		return false;
	};

	switch (target_flags) {
		case 0:
			// actor
			if (target_id > 0) {
				executeOperationSingle(Main_Data::game_actors->GetActor(target_id), Actor);
			}
			break;
		case 1:
			// party member
			executeOperationSingle(Main_Data::game_party->GetActor(target_id), Member);
			break;
		case 2:
		{
			// entire party
			auto count = 0;
			for (Game_Battler* member : Main_Data::game_party->GetActors()) {
				count += executeOperationMany(member);
			}
			Main_Data::game_variables->Set(information_identifier, count);
			break;
		}
		case 3:
			// troop member
			if (target_id > 0) {
				executeOperationSingle(Main_Data::game_enemyparty->GetEnemy(target_id), Enemy);
			}
			break;
		case 4:
		{
			// entire troop
			auto count = 0;
			for (Game_Battler* member : Main_Data::game_enemyparty->GetEnemies()) {
				count += executeOperationMany(member);
			}
			Main_Data::game_variables->Set(information_identifier, count);
			break;
		}
	}

	return true;
}

