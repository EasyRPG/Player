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

#include "battle_battler.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_battle.h"
#include "game_temp.h"
#include "game_interpreter_battle.h"

///////////////////////////////////////////////////////////
Game_Interpreter_Battle::Game_Interpreter_Battle(int depth, bool main_flag) :
	Game_Interpreter(depth, main_flag) {
}

Game_Interpreter_Battle::~Game_Interpreter_Battle() {
}

////////////////////////////////////////////////////////////
/// Execute Command
////////////////////////////////////////////////////////////
bool Game_Interpreter_Battle::ExecuteCommand() {
	if (index >= list.size()) {
		return CommandEnd();
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
			return CommandConditionalBranch(com);
		case Cmd::ElseBranch_B:
			return SkipTo(Cmd::EndBranch_B);
		case Cmd::EndBranch_B:
			return true;
		default:
			return Game_Interpreter::ExecuteCommand();
	}
}

///////////////////////////////////////////////////////////
/// Commands
///////////////////////////////////////////////////////////

bool Game_Interpreter_Battle::CommandCallCommonEvent(RPG::EventCommand const& com) {
	if (child_interpreter != NULL)
		return false;

	int event_id = com.parameters[0];

	const RPG::CommonEvent& event = Data::commonevents[event_id - 1];

	child_interpreter.reset(new Game_Interpreter_Battle(depth + 1));
	child_interpreter->Setup(event.event_commands, 0, event.ID, -2);

	return true;
}

bool Game_Interpreter_Battle::CommandForceFlee(RPG::EventCommand const& com) {
	bool check = com.parameters[2] == 0;

	switch (com.parameters[0]) {
	case 0:
		if (!check || Game_Temp::battle_mode != Game_Temp::BattlePincer)
		Game_Battle::allies_flee = true;
	    break;
	case 1:
		if (!check || Game_Temp::battle_mode != Game_Temp::BattleSurround)
			Game_Battle::MonstersFlee();
	    break;
	case 2:
		if (!check || Game_Temp::battle_mode != Game_Temp::BattleSurround)
			Game_Battle::MonsterFlee(com.parameters[1]);
	    break;
	}

	return true;
}

bool Game_Interpreter_Battle::CommandEnableCombo(RPG::EventCommand const& com) {
	Battle::Ally* ally = Game_Battle::FindAlly(com.parameters[0]);
	if (!ally)
		return true;

	int command_id = com.parameters[1];
	int multiple = com.parameters[2];

	ally->EnableCombo(command_id, multiple);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterHP(RPG::EventCommand const& com) {
	int id = com.parameters[0];
	Battle::Enemy& enemy = Game_Battle::GetEnemy(id);
	bool lose = com.parameters[1] > 0;
	int hp = enemy.GetActor()->GetHp();

	int change;
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

	hp += change;
	if (com.parameters[4] && hp <= 0)
		hp = 1;

	enemy.GetActor()->SetHp(hp);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterMP(RPG::EventCommand const& com) {
	int id = com.parameters[0];
	Battle::Enemy& enemy = Game_Battle::GetEnemy(id);
	bool lose = com.parameters[1] > 0;
	int sp = enemy.GetActor()->GetSp();

	int change;
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

	enemy.GetActor()->SetSp(sp);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterCondition(RPG::EventCommand const& com) {
	Battle::Enemy& enemy = Game_Battle::GetEnemy(com.parameters[0]);
	bool remove = com.parameters[1] > 0;
	int state_id = com.parameters[2];
	if (remove)
		enemy.GetActor()->RemoveState(state_id);
	else
		enemy.GetActor()->AddState(state_id);
	return true;
}

bool Game_Interpreter_Battle::CommandShowHiddenMonster(RPG::EventCommand const& com) {
	Battle::Enemy& enemy = Game_Battle::GetEnemy(com.parameters[0]);
	enemy.game_enemy->SetHidden(false);
	return true;
}

bool Game_Interpreter_Battle::CommandChangeBattleBG(RPG::EventCommand const& com) {
	Game_Battle::ChangeBackground(com.string);
	return true;
}

bool Game_Interpreter_Battle::CommandShowBattleAnimation(RPG::EventCommand const& com) {
	int animation_id = com.parameters[0];
	int target = com.parameters[1];
	bool wait = com.parameters[2] != 0;
	bool allies = com.parameters[3] != 0;
	Battle::Ally* ally = (allies && target >= 0) ? Game_Battle::FindAlly(target) : NULL;
	Battle::Enemy* enemy = (!allies && target >= 0) ? &Game_Battle::GetEnemy(target) : NULL;

	if (active)
		return !Game_Battle::GetScene()->IsAnimationWaiting();

	Game_Battle::GetScene()->ShowAnimation(animation_id, allies, ally, enemy, wait);
	return !wait;
}

bool Game_Interpreter_Battle::CommandTerminateBattle(RPG::EventCommand const& /* com */) {
	Game_Battle::Terminate();
	return true;
}

////////////////////////////////////////////////////////////
/// Conditional Branch
////////////////////////////////////////////////////////////
bool Game_Interpreter_Battle::CommandConditionalBranch(RPG::EventCommand const& com) {
	bool result = false;
	int value1, value2;
	Battle::Ally* ally;

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
		case 2:
			// Hero can act
			ally = Game_Battle::FindAlly(com.parameters[1]);
			result = (ally != NULL && ally->CanAct());
			break;
		case 3:
			// Monster can act
			result = Game_Battle::GetEnemy(com.parameters[1]).CanAct();
			break;
		case 4:
			// Monster is the current target
			result = Game_Battle::HaveTargetEnemy() &&
				Game_Battle::GetTargetEnemy().ID == com.parameters[1];
			break;
		case 5:
			// Hero uses the ... command
			ally = Game_Battle::FindAlly(com.parameters[1]);
			result = (ally != NULL && ally->last_command == com.parameters[2]);
			break;
	}

	if (result)
		return true;

	return SkipTo(Cmd::ElseBranch_B, Cmd::EndBranch_B);
}
