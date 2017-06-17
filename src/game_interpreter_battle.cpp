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
#include "output.h"
#include "player.h"
#include "game_temp.h"

Game_Interpreter_Battle::Game_Interpreter_Battle(int depth, bool main_flag) :
	Game_Interpreter(depth, main_flag) {
}

// Execute Command.
bool Game_Interpreter_Battle::ExecuteCommand() {
	if (index >= list.size()) {
		return CommandEnd();
	}

	if (Game_Battle::IsBattleAnimationWaiting()) {
		return false;
	}

	RPG::EventCommand const& com = list[index];

	switch (com.code) {
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
			return SkipTo(Cmd::EndBranch_B);
		case Cmd::EndBranch_B:
			return true;
		default:
			return Game_Interpreter::ExecuteCommand();
	}
}

// Commands

bool Game_Interpreter_Battle::CommandCallCommonEvent(RPG::EventCommand const& com) {
	if (child_interpreter)
		return false;

	int event_id = com.parameters[0];

	const RPG::CommonEvent& event = Data::commonevents[event_id - 1];

	child_interpreter.reset(new Game_Interpreter_Battle(depth + 1));
	child_interpreter->Setup(event.event_commands, 0, false, event.ID, -2);

	return true;
}

bool Game_Interpreter_Battle::CommandForceFlee(RPG::EventCommand const& com) {
	bool check = com.parameters[2] == 0;
	bool result = false;

	switch (com.parameters[0]) {
	case 0:
		if (!check || Game_Battle::GetBattleMode() != Game_Battle::BattlePincer) {
			Game_Temp::battle_result = Game_Temp::BattleEscape;
			Game_Battle::Terminate();
			result = true;
		}
	    break;
	case 1:
		if (!check || Game_Battle::GetBattleMode() != Game_Battle::BattleSurround) {
			for (int i = 0; i < Main_Data::game_enemyparty->GetBattlerCount(); ++i) {
				Game_Enemy& enemy = (*Main_Data::game_enemyparty)[i];
				enemy.Kill();
			}
			Game_Battle::SetNeedRefresh(true);
			result = true;
		}
	    break;
	case 2:
		if (!check || Game_Battle::GetBattleMode() != Game_Battle::BattleSurround) {
			Game_Enemy& enemy = (*Main_Data::game_enemyparty)[com.parameters[1]];
			enemy.Kill();
			Game_Battle::SetNeedRefresh(true);
			result = true;
		}
	    break;
	}

	if (result) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Escape));
	}

	return true;
}

bool Game_Interpreter_Battle::CommandEnableCombo(RPG::EventCommand const& com) {
	int actor_id = com.parameters[0];

	if (!Main_Data::game_party->IsActorInParty(actor_id)) {
		return true;
	}

	int command_id = com.parameters[1];
	int multiple = com.parameters[2];

	Game_Actors::GetActor(actor_id)->SetBattleCombo(command_id, multiple);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterHP(RPG::EventCommand const& com) {
	int id = com.parameters[0];
	Game_Enemy& enemy = (*Main_Data::game_enemyparty)[id];
	bool lose = com.parameters[1] > 0;
	int hp = enemy.GetHp();

	int change = 0;
	switch (com.parameters[2]) {
	case 0:
		change = com.parameters[3];
	    break;
	case 1:
		change = Game_Variables[com.parameters[3]];
	    break;
	case 2:
		change = com.parameters[3] * hp / 100;
	    break;
	}

	if (lose)
		change = -change;

	enemy.ChangeHp(change);

	if (enemy.IsDead()) {
		Game_Battle::SetNeedRefresh(true);
	}

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterMP(RPG::EventCommand const& com) {
	int id = com.parameters[0];
	Game_Enemy& enemy = (*Main_Data::game_enemyparty)[id];
	bool lose = com.parameters[1] > 0;
	int sp = enemy.GetSp();

	int change = 0;
	switch (com.parameters[2]) {
	case 0:
		change = com.parameters[3];
	    break;
	case 1:
		change = Game_Variables[com.parameters[3]];
	    break;
	}

	if (lose)
		change = -change;

	sp += change;

	enemy.SetSp(sp);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterCondition(RPG::EventCommand const& com) {
	Game_Enemy& enemy = (*Main_Data::game_enemyparty)[com.parameters[0]];
	bool remove = com.parameters[1] > 0;
	int state_id = com.parameters[2];
	if (remove) {
		enemy.RemoveState(state_id);
	} else {
		if (state_id == 1) {
			enemy.ChangeHp(-enemy.GetHp());
			Game_Battle::SetNeedRefresh(true);
		}
		enemy.AddState(state_id);
	}
	return true;
}

bool Game_Interpreter_Battle::CommandShowHiddenMonster(RPG::EventCommand const& com) {
	Game_Enemy& enemy = (*Main_Data::game_enemyparty)[com.parameters[0]];
	enemy.SetHidden(false);
	return true;
}

bool Game_Interpreter_Battle::CommandChangeBattleBG(RPG::EventCommand const& com) {
	Game_Battle::ChangeBackground(com.string);
	return true;
}

bool Game_Interpreter_Battle::CommandShowBattleAnimation(RPG::EventCommand const& com) {
	if (waiting_battle_anim) {
		waiting_battle_anim = Game_Battle::IsBattleAnimationWaiting();
		return !waiting_battle_anim;
	}

	int animation_id = com.parameters[0];
	int target = com.parameters[1];
	waiting_battle_anim = com.parameters[2] != 0;
	bool allies = false;

	if (Player::IsRPG2k3()) {
		allies = com.parameters[3] != 0;
	}

	if (target < 0) {
		std::vector<Game_Battler*> v;

		if (allies) {
			Main_Data::game_party->GetActiveBattlers(v);
		} else {
			Main_Data::game_enemyparty->GetActiveBattlers(v);
		}

		Game_Battle::ShowBattleAnimation(animation_id, v, false);

		return !waiting_battle_anim;
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

		if (!battler_target) {
			return !waiting_battle_anim;
		}

		Game_Battle::ShowBattleAnimation(animation_id, battler_target);
	}

	return !waiting_battle_anim;
}

bool Game_Interpreter_Battle::CommandTerminateBattle(RPG::EventCommand const& /* com */) {
	Game_Battle::Terminate();
	return false;
}

// Conditional branch.
bool Game_Interpreter_Battle::CommandConditionalBranchBattle(RPG::EventCommand const& com) {
	bool result = false;
	int value1, value2;

	switch (com.parameters[0]) {
		case 0:
			// Switch
			result = Game_Switches[com.parameters[1]] == (com.parameters[2] == 0);
			break;
		case 1:
			// Variable
			value1 = Game_Variables[com.parameters[1]];
			if (com.parameters[2] == 0) {
				value2 = com.parameters[3];
			} else {
				value2 = Game_Variables[com.parameters[3]];
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
			Game_Actor* actor = Game_Actors::GetActor(com.parameters[1]);
			if (actor) {
				result = actor->CanAct();
			}
			break;
		}
		case 3:
			// Monster can act
			if (com.parameters[1] < Main_Data::game_enemyparty->GetBattlerCount()) {
				result = (*Main_Data::game_enemyparty)[com.parameters[1]].CanAct();
			}
			break;
		case 4:
			// Monster is the current target
			result = Game_Battle::GetEnemyTargetIndex() == com.parameters[1];
			break;
		case 5:
			// Hero uses the ... command
			result = Game_Actors::GetActor(com.parameters[1])->GetLastBattleAction() == com.parameters[2];
			break;
	}

	if (result)
		return true;

	return SkipTo(Cmd::ElseBranch_B, Cmd::EndBranch_B);
}
