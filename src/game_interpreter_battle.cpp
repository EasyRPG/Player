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
		CommandEnd();
		return true;
	}
	
	switch (list[index].code) {
		case CallCommonEvent:
			return CommandCallCommonEvent();
		case ForceFlee:
			return CommandForceFlee();
		case EnableCombo:
			return CommandEnableCombo();
		case ChangeMonsterHP:
			return CommandChangeMonsterHP();
		case ChangeMonsterMP:
			return CommandChangeMonsterMP();
		case ChangeMonsterCondition:
			return CommandChangeMonsterCondition();
		case ShowHiddenMonster:
			return CommandShowHiddenMonster();
		case ChangeBattleBG:
			return CommandChangeBattleBG();
		case ShowBattleAnimation_B:
			return CommandShowBattleAnimation();
		case TerminateBattle:
			return CommandTerminateBattle();
		case ConditionalBranch_B: 
			return CommandConditionalBranch();
		case ElseBranch:
			return SkipTo(EndBranch);
		case EndBranch:
			return true;
		default:
			return Game_Interpreter::ExecuteCommand();
	}
}

///////////////////////////////////////////////////////////
/// Commands
///////////////////////////////////////////////////////////

bool Game_Interpreter_Battle::CommandCallCommonEvent() {
	if (child_interpreter != NULL)
		return false;

	int event_id = list[index].parameters[0];

	const RPG::CommonEvent& event = Data::commonevents[event_id - 1];

	child_interpreter = new Game_Interpreter_Battle(depth + 1);
	child_interpreter->Setup(event.event_commands, 0, event.ID, -2);

	return true;
}

bool Game_Interpreter_Battle::CommandForceFlee() {
	bool check = list[index].parameters[2] == 0;

	switch (list[index].parameters[0]) {
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
			Game_Battle::MonsterFlee(list[index].parameters[1]);
	    break;
	}

	return true;
}

bool Game_Interpreter_Battle::CommandEnableCombo() {
	Battle::Ally* ally = Game_Battle::FindAlly(list[index].parameters[0]);
	if (!ally)
		return true;

	int command_id = list[index].parameters[1];
	int multiple = list[index].parameters[2];

	ally->EnableCombo(command_id, multiple);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterHP() {
	int id = list[index].parameters[0];
	Battle::Enemy& enemy = Game_Battle::GetEnemy(id);
	bool lose = list[index].parameters[1] > 0;
	int hp = enemy.GetActor()->GetHp();

	int change;
	switch (list[index].parameters[2]) {
	case 0:
		change = list[index].parameters[3];
	    break;
	case 1:
		change = Game_Variables[list[index].parameters[3]];
	    break;
	case 2:
		change = list[index].parameters[3] * hp / 100;
	    break;
	}

	if (lose)
		change = -change;

	hp += change;
	if (list[index].parameters[4] && hp <= 0)
		hp = 1;

	enemy.GetActor()->SetHp(hp);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterMP() {
	int id = list[index].parameters[0];
	Battle::Enemy& enemy = Game_Battle::GetEnemy(id);
	bool lose = list[index].parameters[1] > 0;
	int sp = enemy.GetActor()->GetSp();

	int change;
	switch (list[index].parameters[2]) {
	case 0:
		change = list[index].parameters[3];
	    break;
	case 1:
		change = Game_Variables[list[index].parameters[3]];
	    break;
	}

	if (lose)
		change = -change;

	sp += change;

	enemy.GetActor()->SetSp(sp);

	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterCondition() {
	Battle::Enemy& enemy = Game_Battle::GetEnemy(list[index].parameters[0]);
	bool remove = list[index].parameters[1] > 0;
	int state_id = list[index].parameters[2];
	if (remove)
		enemy.GetActor()->RemoveState(state_id);
	else
		enemy.GetActor()->AddState(state_id);
	return true;
}

bool Game_Interpreter_Battle::CommandShowHiddenMonster() {
	Battle::Enemy& enemy = Game_Battle::GetEnemy(list[index].parameters[0]);
	enemy.game_enemy->SetHidden(false);
	return true;
}

bool Game_Interpreter_Battle::CommandChangeBattleBG() {
	Game_Battle::ChangeBackground(list[index].string);
	return true;
}

bool Game_Interpreter_Battle::CommandShowBattleAnimation() {
	int animation_id = list[index].parameters[0];
	int target = list[index].parameters[1];
	bool wait = list[index].parameters[2] != 0;
	bool allies = list[index].parameters[3] != 0;
	Battle::Ally* ally = (allies && target >= 0) ? Game_Battle::FindAlly(target) : NULL;
	Battle::Enemy* enemy = (!allies && target >= 0) ? &Game_Battle::GetEnemy(target) : NULL;

	if (active)
		return !Game_Battle::IsAnimationWaiting();

	Game_Battle::ShowAnimation(animation_id, allies, ally, enemy, wait);
	return !wait;
}

bool Game_Interpreter_Battle::CommandTerminateBattle() {
	Game_Battle::Terminate();
	return true;
}

////////////////////////////////////////////////////////////
/// Conditional Branch
////////////////////////////////////////////////////////////
bool Game_Interpreter_Battle::CommandConditionalBranch() {
	bool result = false;
	int value1, value2;
	Battle::Ally* ally;

	switch (list[index].parameters[0]) {
		case 0:
			// Switch
			result = Game_Switches[list[index].parameters[1]] == (list[index].parameters[2] == 0);
			break;
		case 1:
			// Variable
			value1 = Game_Variables[list[index].parameters[1]];
			if (list[index].parameters[2] == 0) {
				value2 = list[index].parameters[3];
			} else {
				value2 = Game_Variables[list[index].parameters[3]];
			}
			switch (list[index].parameters[4]) {
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
			ally = Game_Battle::FindAlly(list[index].parameters[1]);
			result = (ally != NULL && ally->CanAct());
			break;
		case 3:
			// Monster can act
			result = Game_Battle::GetEnemy(list[index].parameters[1]).CanAct();
			break;
		case 4:
			// Monster is the current target
			result = Game_Battle::HaveTargetEnemy() &&
				Game_Battle::GetTargetEnemy().ID == list[index].parameters[1];
			break;
		case 5:
			// Hero uses the ... command
			ally = Game_Battle::FindAlly(list[index].parameters[1]);
			result = (ally != NULL && ally->last_command == list[index].parameters[2]);
			break;
	}

	if (result)
		return true;

	return SkipTo(ElseBranch, EndBranch);
}

