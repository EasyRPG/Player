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
	return true;
}

bool Game_Interpreter_Battle::CommandForceFlee() {
	return true;
}

bool Game_Interpreter_Battle::CommandEnableCombo() {
	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterHP() {
	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterMP() {
	return true;
}

bool Game_Interpreter_Battle::CommandChangeMonsterCondition() {
	return true;
}

bool Game_Interpreter_Battle::CommandShowHiddenMonster() {
	return true;
}

bool Game_Interpreter_Battle::CommandChangeBattleBG() {
	return true;
}

bool Game_Interpreter_Battle::CommandShowBattleAnimation() {
	return true;
}

bool Game_Interpreter_Battle::CommandTerminateBattle() {
	return true;
}

////////////////////////////////////////////////////////////
/// Conditional Branch
////////////////////////////////////////////////////////////
bool Game_Interpreter_Battle::CommandConditionalBranch() {
	bool result = false;
	int value1, value2;
	Battle::Ally* ally;
	Battle::Enemy* enemy;

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
			// Hero (ID == [1]) can act
			// FIXME
			break;
		case 3:
			// Monster (index == [1]) can act
			// FIXME
			break;
		case 4:
			// Monster (index == [1]) is the current target
			// FIXME
			break;
		case 5:
			// Hero (ID == [1]) uses the [2] command
			// FIXME
			break;
	}

	if (result)
		return true;

	return SkipTo(ElseBranch, EndBranch);
}

